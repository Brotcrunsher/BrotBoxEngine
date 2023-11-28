#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <Windows.h>
#include "AssetStore.h"

//TODO: GATW: kill (?) Time Wasters Processes during working hours and while still tasks are open.
//TODO: Add "fixed date" tasks. "Every month/year at this and that date". Useful e.g. for Taxes.
//TODO: Butchered looks on non 4k
//TODO: [MIN]:[SEC] looks dumb when sec <= 9 cause sec then only has one digit.

#define WM_SYSICON        (WM_USER + 1)
#define ID_EXIT           1002

NOTIFYICONDATA notifyIconData;
HWND Hwnd;
HMENU Hmenu;
HICON hIcon;
char szClassName[] = "MyWindowClassName";
char szTIP[64] = "M.O.THE.R " __DATE__ ", " __TIME__;
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

class MyGame;
MyGame* myGame;
bool exitRequested = false;

struct Task
{
	enum class InputType
	{
		NONE,
		INTEGER,
		FLOAT,
	};
	static constexpr const char* inputTypeItems[] = { "None", "Integer", "Float" };
	static InputType strToInputType(const char* str)
	{
		if (strcmp(str, inputTypeItems[0]) == 0) return InputType::NONE;
		if (strcmp(str, inputTypeItems[1]) == 0) return InputType::INTEGER;
		if (strcmp(str, inputTypeItems[2]) == 0) return InputType::FLOAT;
		throw bbe::IllegalStateException();
	}
	static const char* inputTypeToStr(InputType it)
	{
		if (it == InputType::NONE)    return inputTypeItems[0];
		if (it == InputType::INTEGER) return inputTypeItems[1];
		if (it == InputType::FLOAT)   return inputTypeItems[2];
		throw bbe::IllegalStateException();
	}

	char title[1024] = {};
	int32_t repeatDays = 0;
	bbe::TimePoint previousExecution;
private:
	bbe::TimePoint nextExecution; // Call nextPossibleExecution from the outside! 
public:
	bool canBeSundays = true;
	int32_t followUp = 0; // In minutes. When clicking follow up, the task will be rescheduled the same day.
	int32_t internalValue = 0;
	int32_t internalValueIncrease = 0;
	int32_t followUp2 = 0;
	InputType inputType = InputType::NONE;
	bbe::List<float> history;

	// Non-Persisted Helper Data below.
	const char* inputTypeStr = inputTypeItems[0];
	int32_t inputInt = 0;
	float inputFloat = 0;
	bool armedToPlaySound = false;

	bool shouldPlaySound()
	{
		if (nextPossibleExecution().hasPassed())
		{
			if (armedToPlaySound)
			{
				armedToPlaySound = false;
				return true;
			}
		}
		else
		{
			armedToPlaySound = true;
		}
		return false;
	}
	void execDone()
	{
		internalValue += internalValueIncrease;
		previousExecution = bbe::TimePoint();
		nextExecution = previousExecution.nextMorning().plusDays(repeatDays - 1);
		if (!canBeSundays && nextExecution.isSunday())
		{
			nextExecution = nextExecution.plusDays(1);
		}
	}
	void execFollowUp()
	{
		previousExecution = bbe::TimePoint();
		nextExecution = previousExecution.plusMinutes(followUp);
	}
	void execFollowUp2()
	{
		previousExecution = bbe::TimePoint();
		nextExecution = previousExecution.plusMinutes(followUp2);
	}
	void execMoveToNow()
	{
		armedToPlaySound = false;
		nextExecution = bbe::TimePoint();
	}

	void sanity()
	{
		if (repeatDays < 1) repeatDays = 1;
		if (followUp < 0) followUp = 0;
	}
	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.writeNullString(title);
		buffer.write(repeatDays);
		previousExecution.serialize(buffer);
		nextExecution.serialize(buffer);
		buffer.write(canBeSundays);
		buffer.write(followUp);
		buffer.write(internalValue);
		buffer.write(internalValueIncrease);
		buffer.write(followUp2);
		buffer.write((int32_t)inputType);
		buffer.write(history);
	}
	static Task deserialize(bbe::ByteBufferSpan& buffer)
	{
		Task retVal;

		strcpy(retVal.title, buffer.readNullString());
		buffer.read(retVal.repeatDays);
		retVal.previousExecution = bbe::TimePoint::deserialize(buffer);
		retVal.nextExecution = bbe::TimePoint::deserialize(buffer);
		buffer.read(retVal.canBeSundays, true);
		buffer.read(retVal.followUp);
		buffer.read(retVal.internalValue);
		buffer.read(retVal.internalValueIncrease);
		buffer.read(retVal.followUp2);
		int32_t inputType = 0;
		buffer.read(inputType);
		retVal.inputType = (InputType)inputType;
		retVal.inputTypeStr = inputTypeToStr(retVal.inputType);
		buffer.read(retVal.history);

		return retVal;
	}

	bbe::TimePoint nextPossibleExecution() const
	{
		if (!nextExecution.hasPassed()) return nextExecution;
		bbe::TimePoint execTime;
		if (!canBeSundays && execTime.isSunday()) execTime = execTime.nextMorning();
		return execTime;
	}

	bool isImportantTomorrow() const
	{
		bbe::TimePoint tomorrow = bbe::TimePoint().plusDays(1);
		if (nextExecution < tomorrow) return true;
		if (repeatDays <= 1) return true;

		return false;
	}
};

class MyGame : public bbe::Game
{
private:
	bbe::SerializableList<Task> tasks = bbe::SerializableList<Task>("config.dat", "ParanoiaConfig");
	bool editMode = false;
	bool shiftPressed = false;

	bbe::List<HICON> trayIconsRed;
	bbe::List<HICON> trayIconsGreen;
	bbe::List<HICON> trayIconsBlue;
	size_t trayIconIndex = 0;

	int32_t amountOfTasksNow = 0;

public:
	MyGame()
	{
		myGame = this;
	}

	HICON createTrayIcon(DWORD offset, int redGreenBlue)
	{
		// See: https://learn.microsoft.com/en-us/windows/win32/menurc/using-cursors#creating-a-cursor
		constexpr DWORD iconWidth = 32;
		constexpr DWORD iconHeight = 32;

		BITMAPV5HEADER bi = {};
		bi.bV5Size = sizeof(BITMAPV5HEADER);
		bi.bV5Width = iconWidth;
		bi.bV5Height = iconHeight;
		bi.bV5Planes = 1;
		bi.bV5BitCount = 32;
		bi.bV5Compression = BI_BITFIELDS;
		// The following mask specification specifies a supported 32 BPP
		// alpha format for Windows XP.
		bi.bV5RedMask = 0x00FF0000;
		bi.bV5GreenMask = 0x0000FF00;
		bi.bV5BlueMask = 0x000000FF;
		bi.bV5AlphaMask = 0xFF000000;

		// Create the DIB section with an alpha channel.
		HDC hdc = GetDC(NULL);
		void* lpBits;
		HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
			&lpBits, NULL, (DWORD)0);
		ReleaseDC(NULL, hdc);

		// Create an empty mask bitmap.
		HBITMAP hMonoBitmap = CreateBitmap(iconWidth, iconHeight, 1, 1, NULL);

		// Set the alpha values for each pixel in the cursor so that
		// the complete cursor is semi-transparent.
		DWORD* lpdwPixel = (DWORD*)lpBits;
		constexpr DWORD centerX = iconWidth / 2;
		constexpr DWORD centerY = iconHeight / 2;
		for (DWORD x = 0; x < iconWidth; x++)
			for (DWORD y = 0; y < iconHeight; y++)
			{
				const DWORD xDiff = x - centerX;
				const DWORD yDiff = y - centerY;
				const DWORD distSq = xDiff * xDiff + yDiff * yDiff;
				const DWORD dist = bbe::Math::sqrt(distSq * 1000) + offset;
				const DWORD cVal = dist % 512;

				const DWORD highlight = cVal > 255 ? 255 : cVal;
				const DWORD white = cVal > 255 ? (cVal - 255) : 0;

				*lpdwPixel = 0;

				if (redGreenBlue == 0) /*Red*/
				{
					*lpdwPixel |= white;
					*lpdwPixel |= white << 8;
					*lpdwPixel |= highlight << 16;
				}
				else if (redGreenBlue == 1) /*Green*/
				{
					*lpdwPixel |= white;
					*lpdwPixel |= highlight << 8;
					*lpdwPixel |= white << 16;
				}
				else if (redGreenBlue == 2) /*Blue*/
				{
					*lpdwPixel |= highlight;
					*lpdwPixel |= white << 8;
					*lpdwPixel |= white << 16;
				}
				else
				{
					throw bbe::IllegalArgumentException();
				}

				*lpdwPixel |= 0xFF << 24;

				lpdwPixel++;
			}

		ICONINFO ii = {};
		ii.fIcon = TRUE;
		ii.hbmMask = hMonoBitmap;
		ii.hbmColor = hBitmap;

		// Create the alpha cursor with the alpha DIB section.
		HICON hAlphaCursor = CreateIconIndirect(&ii);

		DeleteObject(hBitmap);
		DeleteObject(hMonoBitmap);

		return hAlphaCursor;
	}

	void createTrayIcons()
	{
		for (DWORD offset = 0; offset < 512; offset ++)
		{
			trayIconsRed.add(createTrayIcon(offset, 0));
			trayIconsGreen.add(createTrayIcon(offset, 1));
			trayIconsBlue.add(createTrayIcon(offset, 2));
		}
	}

	bbe::List<HICON>& getTrayIcons()
	{
		if (isNightTime()) return trayIconsRed;
		if (amountOfTasksNow > 0) return trayIconsBlue;
		return trayIconsGreen;
	}

	HICON getCurrentTrayIcon()
	{
		bbe::List<HICON>& trayIcons = getTrayIcons();
		HICON retVal = trayIcons[((int)(getTimeSinceStartSeconds() * 400)) % trayIcons.getLength()];
		return retVal;
	}

	virtual void onStart() override
	{
		setWindowCloseMode(bbe::WindowCloseMode::HIDE);
		setTargetFrametime(1.f / 144.f);

		WNDCLASSEX wincl = {};        /* Data structure for the windowclass */
		wincl.hInstance = GetModuleHandle(0);
		wincl.lpszClassName = szClassName;
		wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
		wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
		wincl.cbSize = sizeof(WNDCLASSEX);

		/* Use default icon and mouse-pointer */
		//wincl.hIcon = hIcon;
		//wincl.hIconSm = hIcon;
		wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wincl.lpszMenuName = NULL;                 /* No menu */
		wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
		wincl.cbWndExtra = 0;                      /* structure or the window instance */
		wincl.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
		RegisterClassEx(&wincl);


		/* The class is registered, let's create the program*/
		Hwnd = CreateWindowEx(
			0,                   /* Extended possibilites for variation */
			szClassName,         /* Classname */
			szClassName,       /* Title Text */
			WS_OVERLAPPEDWINDOW, /* default window */
			CW_USEDEFAULT,       /* Windows decides the position */
			CW_USEDEFAULT,       /* where the window ends up on the screen */
			544,                 /* The programs width */
			375,                 /* and height in pixels */
			HWND_DESKTOP,        /* The window is a child-window to desktop */
			NULL,                /* No menu */
			GetModuleHandle(0),       /* Program Instance handler */
			NULL                 /* No Window Creation data */
		);

		Hmenu = CreatePopupMenu();
		AppendMenu(Hmenu, MF_STRING, ID_EXIT, TEXT("Exit"));

		createTrayIcons();
		setCurrentTrayIcon(true);
	}

	void setCurrentTrayIcon(bool firstCall)
	{
		memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

		notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = Hwnd;
		notifyIconData.uID = 0;
		notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
		notifyIconData.hIcon = getCurrentTrayIcon();
		memcpy_s(notifyIconData.szTip, sizeof(notifyIconData.szTip), szTIP, sizeof(szTIP));

		Shell_NotifyIcon(firstCall ? NIM_ADD : NIM_MODIFY, &notifyIconData);
	}

	bbe::TimePoint getNightStart()
	{
		// TODO: This is something highly personalized for my own current usage. It probably needs to be removed some day.
		//       It takes away one minute for every passed day since 2023/11/22. Slowly approaching a more healthy sleep
		//       schedule =)
		const bbe::TimePoint qualifyingDate = bbe::TimePoint::fromDate(2023, bbe::Month::NOVEMBER, 22);
		const bbe::Duration timeSinceQualifyingDate = bbe::TimePoint() - qualifyingDate;
		int32_t daysSinceQualifyingDate = timeSinceQualifyingDate.toDays();
		if (daysSinceQualifyingDate > 60) daysSinceQualifyingDate = 60;

		return bbe::TimePoint::todayAt(23, 59 - daysSinceQualifyingDate);
	}

	bool isNightTime()
	{
		bbe::TimePoint now;
		return bbe::TimePoint::todayAt(5, 00) > now || now > getNightStart();
	}

	bool shouldPlayAlmostNightWarning()
	{
		static bool playedBefore = false;
		if (!playedBefore)
		{
			bbe::TimePoint now;
			if (now > getNightStart().plusMinutes(-5))
			{
				playedBefore = true;
				return true;
			}
		}
		return false;
	}

	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::E)) editMode = !editMode;
		shiftPressed = isKeyDown(bbe::Key::LEFT_SHIFT);

		bool playSound = false;
		for (size_t i = 0; i < tasks.getLength(); i++)
		{
			playSound |= tasks[i].shouldPlaySound();
		}
		if (playSound)
		{
			assetStore::NewTask()->play();
		}
		if (exitRequested)
		{
			closeWindow();
		}

		setCurrentTrayIcon(false);

		if (isNightTime())
		{
			static float timeSinceLastMinimize = 100000.0f;
			timeSinceLastMinimize += timeSinceLastFrame;
			if (timeSinceLastMinimize > 60.0f)
			{
				timeSinceLastMinimize = 0.0f;
				HWND hwnd = FindWindow("Shell_TrayWnd", NULL);
				LRESULT res = SendMessage(hwnd, WM_COMMAND, (WPARAM)419, 0);
				showWindow();
				assetStore::NightTime()->play();
			}
		}
		if (shouldPlayAlmostNightWarning())
		{
			assetStore::AlmostNightTime()->play();
		}
	}

	int32_t drawTable(const char* title, const std::function<bool(Task&)>& predicate, bool& contentsChanged, bool showMoveToNow, bool showCountdown, bool showDone, bool showFollowUp, bool highlightRareTasks)
	{
		int32_t amountDrawn = 0;
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), title);
		if (ImGui::BeginTable("table2", 6, ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 400);
			if(showCountdown) ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
			ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("EEE", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("FFF", ImGuiTableColumnFlags_WidthStretch);
			for (size_t i = 0; i < tasks.getLength(); i++)
			{
				Task& t = tasks[i];
				if (!predicate(t)) continue;
				amountDrawn++;
				ImGui::PushID(i);
				ImGui::TableNextRow();
				int32_t column = 0;
				ImGui::TableSetColumnIndex(column++);
				if (highlightRareTasks && t.repeatDays > 1)
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(!)");
					ImGui::SameLine();
				}
				bbe::String modifiedTitle = t.title;
				if (modifiedTitle.contains("[MIN]"))
				{
					auto value = t.internalValue / 60;
					modifiedTitle = modifiedTitle.replace("[MIN]", bbe::String(value));
				}
				if (modifiedTitle.contains("[SEC]"))
				{
					auto value = t.internalValue % 60;
					modifiedTitle = modifiedTitle.replace("[SEC]", bbe::String(value));
				}
				ImGui::Text(modifiedTitle.getRaw(), t.internalValue);
				if (t.history.getLength() > 1)
				{
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
					{
						ImGui::PlotLines("##History", t.history.getRaw(), t.history.getLength());
						ImGui::EndTooltip();
					}
				}
				if (showCountdown)
				{
					ImGui::TableSetColumnIndex(column++);
					bbe::String s = (t.nextPossibleExecution() - bbe::TimePoint()).toString();
					const char* c = s.getRaw();
					ImGui::SetCursorPosX(
						+ ImGui::GetCursorPosX() 
						+ ImGui::GetColumnWidth() 
						- ImGui::CalcTextSize(c).x
						- ImGui::GetScrollX() 
						- 10 * ImGui::GetStyle().ItemSpacing.x);
					ImGui::Text(c);
				}
				ImGui::TableSetColumnIndex(column++);
				if (showDone)
				{
					if (t.inputType == Task::InputType::NONE)
					{
						if (ImGui::Button("Done"))
						{
							t.execDone();
							contentsChanged = true;
						}
					}
					else if (t.inputType == Task::InputType::INTEGER)
					{
						if (ImGui::InputInt("##input", &t.inputInt, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							t.history.add(t.inputInt);
							t.execDone();
							contentsChanged = true;
						}
					}
					else if (t.inputType == Task::InputType::FLOAT)
					{
						if (ImGui::InputFloat("##input", &t.inputFloat, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							t.history.add(t.inputFloat);
							t.execDone();
							contentsChanged = true;
						}
					}
					else
					{
						throw bbe::IllegalStateException();
					}
				}
				ImGui::TableSetColumnIndex(column++);
				if (showFollowUp && t.followUp > 0 && ImGui::Button((bbe::String("+")+t.followUp+"min").getRaw()))
				{
					t.execFollowUp();
					contentsChanged = true;
				}
				ImGui::TableSetColumnIndex(column++);
				if (showFollowUp && t.followUp2 > 0 && ImGui::Button((bbe::String("+") + t.followUp2 + "min").getRaw()))
				{
					t.execFollowUp2();
					contentsChanged = true;
				}
				if (showMoveToNow)
				{
					ImGui::TableSetColumnIndex(column++);
					if (ImGui::Button("Move to Now"))
					{
						t.execMoveToNow();
						contentsChanged = true;
					}
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();
		return amountDrawn;
	}

	bool drawEditableTask(Task& t)
	{
		bool taskChanged = false;
		taskChanged |= ImGui::InputText("Title", t.title, sizeof(t.title));
		taskChanged |= ImGui::InputInt("Repeat Days", &t.repeatDays);
		taskChanged |= ImGui::Checkbox("Can be Sundays", &t.canBeSundays);
		taskChanged |= ImGui::InputInt("Follow Up  (in Minutes)", &t.followUp);
		taskChanged |= ImGui::InputInt("Follow Up2 (in Minutes)", &t.followUp2);
		taskChanged |= ImGui::InputInt("Internal Value", &t.internalValue);
		taskChanged |= ImGui::InputInt("Internal Value Increase", &t.internalValueIncrease);
		if (ImGui::BeginCombo("Input Type", t.inputTypeStr))
		{
			for (int i = 0; i < IM_ARRAYSIZE(Task::inputTypeItems); i++)
			{
				if (ImGui::Selectable(Task::inputTypeItems[i]))
				{
					t.inputTypeStr = Task::inputTypeItems[i];
					t.inputType = Task::strToInputType(t.inputTypeStr);
					taskChanged = true;
				}
			}
			ImGui::EndCombo();
		}
		return taskChanged;
	}

	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
		if (!editMode)
		{
			ImGuiViewport viewport = *ImGui::GetMainViewport();
			viewport.WorkSize.x *= 0.6f;
			ImGui::SetNextWindowPos(viewport.WorkPos);
			ImGui::SetNextWindowSize(viewport.WorkSize);
			ImGui::Begin("Edit Mode", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
			bool contentsChanged = false;
			amountOfTasksNow = drawTable("Now", [](Task& t) { return t.nextPossibleExecution().hasPassed(); },                                                                      contentsChanged, false, false, true,  true, false);
			drawTable("Today",    [](Task& t) { return !t.nextPossibleExecution().hasPassed() && t.nextPossibleExecution().isToday(); },                              contentsChanged, true,  true,  true,  true, false);
			drawTable("Tomorrow", [](Task& t) { return t.isImportantTomorrow(); },                                                                                    contentsChanged, true, false, false, true, true);
			drawTable("Later",    [](Task& t) { return !t.nextPossibleExecution().hasPassed() && !t.nextPossibleExecution().isToday() && !t.isImportantTomorrow(); }, contentsChanged, true,  true,  true,  true, false);
			if (contentsChanged)
			{
				tasks.writeToFile();
			}
			ImGui::End();

			viewport.WorkPos.x += viewport.WorkSize.x;
			viewport.WorkSize.x *= ImGui::GetMainViewport()->WorkSize.x * 0.4f;
			ImGui::SetNextWindowPos(viewport.WorkPos);
			ImGui::SetNextWindowSize(viewport.WorkSize);
			ImGui::Begin("Info", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text("Build: " __DATE__ ", " __TIME__);
			bbe::String s = "Night Start in: " + (getNightStart() - bbe::TimePoint()).toString();
			ImGui::Text(s.getRaw());
			ImGui::End();
		}
		else
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::Begin("Edit Mode", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Edit Mode (Hit CTRL+E to leave)");
			ImGui::Separator();
			ImGui::Text("Build: " __DATE__ ", " __TIME__);
			ImGui::Separator();

			{
				static Task tempTask;
				drawEditableTask(tempTask);
				tempTask.sanity();

				if (ImGui::Button("New Task"))
				{
					tempTask.inputType = Task::strToInputType(tempTask.inputTypeStr);
					tasks.add(tempTask);
					tempTask = Task();
				}
			}
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::NewLine();

			bool tasksChanged = false;
			size_t deletionIndex = (size_t)-1;
			for (size_t i = 0; i < tasks.getLength(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::Button(shiftPressed ? "Delete Task" : "[Shift]") && shiftPressed)
				{
					deletionIndex = i;
				}
				if (i != 0)
				{
					ImGui::SameLine();
					if (ImGui::Button("Up"))
					{
						tasks.swap(i, i - 1);
					}
				}
				if (i != tasks.getLength() - 1)
				{
					ImGui::SameLine();
					if (ImGui::Button("Down"))
					{
						tasks.swap(i, i + 1);
					}
				}
				Task& t = tasks[i];
				tasksChanged |= drawEditableTask(t);
				ImGui::Text(t.previousExecution.toString().getRaw());
				ImGui::Text(t.nextPossibleExecution().toString().getRaw());
				ImGui::NewLine();
				ImGui::Separator();
				ImGui::NewLine();
				ImGui::PopID();
				t.sanity();
			}
			ImGui::End();
			tasks.removeIndex(deletionIndex);
			if (tasksChanged)
			{
				tasks.writeToFile();
			}
		}

		//ImGui::ShowDemoWindow();
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)                  /* handle the messages */
	{
	case WM_SYSICON:
	{
		if (lParam == WM_RBUTTONDOWN)
		{
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns

			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);



			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			if (clicked == ID_EXIT)
			{
				exitRequested = true;
			}
		}
		else if (lParam == WM_LBUTTONDBLCLK)
		{
			myGame->showWindow();
		}
		break;
	}

	case WM_DESTROY:

		PostQuitMessage(0);
		break;

	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int main()
{
	HWND hWnd = GetConsoleWindow(); 
	FreeConsole();
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "M.O.THE.R - Memory of the repetitive");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}