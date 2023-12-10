#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <AtlBase.h>
#include <UIAutomation.h>
#include "AssetStore.h"

//TODO: GATW: Also play "Open Tasks" sound when opening time wasting URLs
//TODO: Add "fixed date" tasks. "Every month/year at this and that date". Useful e.g. for Taxes.
//TODO: Butchered looks on non 4k
//TODO: Implement proper date picker
//TODO: Somehow mark error if all weekdays are marked as impossible
//TODO: Play sound ever hour if ignoreNight and nightActive to ask if I am sure that I wanna stay awake.

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
	bool canBeSu = true;
	int32_t followUp = 0; // In minutes. When clicking follow up, the task will be rescheduled the same day.
	int32_t internalValue = 0;
	int32_t internalValueIncrease = 0;
	int32_t followUp2 = 0;
	InputType inputType = InputType::NONE;
	bbe::List<float> history;
	bool advanceable = false;
	bool oneShot = false;
	bool preparation = false;
	bool canBeMo = true;
	bool canBeTu = true;
	bool canBeWe = true;
	bool canBeTh = true;
	bool canBeFr = true;
	bool canBeSa = true;
	bool earlyAdvanceable = true;

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
		nextExecution = toPossibleTimePoint(previousExecution.nextMorning().plusDays(repeatDays - 1));
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
	void execAdvance()
	{
		if (preparation)
		{
			nextExecution = toPossibleTimePoint(bbe::TimePoint().nextMorning().plusDays(repeatDays));
		}
		else
		{
			nextExecution = toPossibleTimePoint(nextExecution.plusDays(repeatDays));
		}
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
		buffer.write(canBeSu);
		buffer.write(followUp);
		buffer.write(internalValue);
		buffer.write(internalValueIncrease);
		buffer.write(followUp2);
		buffer.write((int32_t)inputType);
		buffer.write(history);
		buffer.write(advanceable);
		buffer.write(oneShot);
		buffer.write(preparation);
		buffer.write(canBeMo);
		buffer.write(canBeTu);
		buffer.write(canBeWe);
		buffer.write(canBeTh);
		buffer.write(canBeFr);
		buffer.write(canBeSa);
		buffer.write(earlyAdvanceable);
	}
	static Task deserialize(bbe::ByteBufferSpan& buffer)
	{
		Task retVal;

		strcpy(retVal.title, buffer.readNullString());
		buffer.read(retVal.repeatDays);
		retVal.previousExecution = bbe::TimePoint::deserialize(buffer);
		retVal.nextExecution = bbe::TimePoint::deserialize(buffer);
		buffer.read(retVal.canBeSu, true);
		buffer.read(retVal.followUp);
		buffer.read(retVal.internalValue);
		buffer.read(retVal.internalValueIncrease);
		buffer.read(retVal.followUp2);
		int32_t inputType = 0;
		buffer.read(inputType);
		retVal.inputType = (InputType)inputType;
		retVal.inputTypeStr = inputTypeToStr(retVal.inputType);
		buffer.read(retVal.history);
		buffer.read(retVal.advanceable);
		buffer.read(retVal.oneShot);
		buffer.read(retVal.preparation);
		buffer.read(retVal.canBeMo, true);
		buffer.read(retVal.canBeTu, true);
		buffer.read(retVal.canBeWe, true);
		buffer.read(retVal.canBeTh, true);
		buffer.read(retVal.canBeFr, true);
		buffer.read(retVal.canBeSa, true);
		buffer.read(retVal.earlyAdvanceable, true);

		return retVal;
	}

	bbe::TimePoint nextPossibleExecution() const
	{
		if (!nextExecution.hasPassed()) return nextExecution;
		return toPossibleTimePoint(bbe::TimePoint());
	}

	bbe::TimePoint toPossibleTimePoint(const bbe::TimePoint& tp) const
	{
		bbe::TimePoint retVal = tp;
		for (int32_t i = 0; i < 2; i++)
		{
			if (!canBeMo && retVal.isMonday())    retVal = retVal.nextMorning();
			if (!canBeTu && retVal.isTuesday())   retVal = retVal.nextMorning();
			if (!canBeWe && retVal.isWednesday()) retVal = retVal.nextMorning();
			if (!canBeTh && retVal.isThursday())  retVal = retVal.nextMorning();
			if (!canBeFr && retVal.isFriday())    retVal = retVal.nextMorning();
			if (!canBeSa && retVal.isSaturday())  retVal = retVal.nextMorning();
			if (!canBeSu && retVal.isSunday())    retVal = retVal.nextMorning();
		}
		return retVal;
	}

	bool isImportantTomorrow() const
	{
		bbe::TimePoint tomorrow = bbe::TimePoint().nextMorning().plusDays(1).plusSeconds(-1);
		if (nextExecution < tomorrow) return true;

		return false;
	}

	bool isImportantToday() const
	{
		auto tp = nextPossibleExecution();
		if (tp.hasPassed()) return true;
		if (tp.isToday()) return true;
		return false;
	}

	void setNextExecution(int32_t year, int32_t month, int32_t day)
	{
		nextExecution = toPossibleTimePoint(bbe::TimePoint::fromDate(year, month, day).nextMorning());
	}
};

struct Process
{
	static constexpr const char* typeStrings[] = { "Unknown", "System", "Other", "Game" };
	static constexpr int32_t TYPE_UNKNOWN = 0;
	static constexpr int32_t TYPE_SYSTEM = 1;
	static constexpr int32_t TYPE_OTHER = 2;
	static constexpr int32_t TYPE_GAME = 3;
	static int32_t strToType(const char* str)
	{
		if (strcmp(str, typeStrings[0]) == 0) return TYPE_UNKNOWN;
		if (strcmp(str, typeStrings[1]) == 0) return TYPE_SYSTEM;
		if (strcmp(str, typeStrings[2]) == 0) return TYPE_OTHER;
		if (strcmp(str, typeStrings[3]) == 0) return TYPE_GAME;
		throw bbe::IllegalStateException();
	}
	static const char* typeToStr(int32_t it)
	{
		if (it == TYPE_UNKNOWN) return typeStrings[0];
		if (it == TYPE_SYSTEM)  return typeStrings[1];
		if (it == TYPE_OTHER)   return typeStrings[2];
		if (it == TYPE_GAME)    return typeStrings[3];
		throw bbe::IllegalStateException();
	}

	char title[1024] = {};
	int32_t type = TYPE_UNKNOWN;


	// Non-Persisted Helper Data below.
	const char* inputTypeStr = typeStrings[0];

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.writeNullString(title);
		buffer.write(type);
	}
	static Process deserialize(bbe::ByteBufferSpan& buffer)
	{
		Process retVal;

		strcpy(retVal.title, buffer.readNullString());
		buffer.read(retVal.type);
		retVal.inputTypeStr = typeToStr(retVal.type);

		return retVal;
	}
};

class MyGame : public bbe::Game
{
private:
	bbe::SerializableList<Task> tasks = bbe::SerializableList<Task>("config.dat", "ParanoiaConfig");
	bbe::SerializableList<Process> processes = bbe::SerializableList<Process>("processes.dat", "ParanoiaConfig");
	bool editMode = false;
	bool shiftPressed = false;
	bool isGameOn = false;
	bool openTasksNotificationSilenced = false;
	bool showDebugStuff = false;
	bool ignoreNight = false;

	bbe::List<HICON> trayIconsRed;
	bbe::List<HICON> trayIconsGreen;
	bbe::List<HICON> trayIconsBlue;
	size_t trayIconIndex = 0;

	int32_t amountOfTasksNow = 0;
	int32_t amountOfTasksNowWithoutOneShot = 0;

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
		static HICON previousHIcon = nullptr;
		const HICON currentHIcon = getCurrentTrayIcon();
		if (previousHIcon == currentHIcon)
		{
			return;
		}
		previousHIcon = currentHIcon;

		memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

		notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = Hwnd;
		notifyIconData.uID = 0;
		notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
		notifyIconData.hIcon = currentHIcon;
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

	bool isLateAdvanceableTime()
	{
		return bbe::TimePoint() > bbe::TimePoint::todayAt(18, 00);
	}

	bool isWorkTime()
	{
		bbe::TimePoint now;
		return now > bbe::TimePoint::todayAt(5, 00) && now < bbe::TimePoint::todayAt(17, 00);
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
		beginMeasure("Basic Controls");
		if (isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::E)) editMode = !editMode;
		shiftPressed = isKeyDown(bbe::Key::LEFT_SHIFT);

		beginMeasure("Play Task Sounds");
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

		beginMeasure("Tray Icon");
		setCurrentTrayIcon(false);

		beginMeasure("Night Time");
		if (!ignoreNight && isNightTime())
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

		beginMeasure("Task Amount Calculation");
		amountOfTasksNow = 0;
		amountOfTasksNowWithoutOneShot = 0;

		for (size_t i = 0; i < tasks.getLength(); i++)
		{
			Task& t = tasks[i];
			if (t.nextPossibleExecution().hasPassed())
			{
				amountOfTasksNow++;
				if (!t.oneShot)
				{
					amountOfTasksNowWithoutOneShot++;
				}
			}
		}

		beginMeasure("Process Stuff");
		static float timeSinceLastProcessStuff = 0;
		timeSinceLastProcessStuff += timeSinceLastFrame;
		if(timeSinceLastProcessStuff > 10.f)
		{
			timeSinceLastProcessStuff = 0.0f;
			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
			PROCESSENTRY32 entry;
			entry.dwSize = sizeof(entry);
			BOOL hasEntry = Process32First(snapshot, &entry);
			isGameOn = false;
			while (hasEntry)
			{
				bool found = false;
				for (size_t i = 0; i < processes.getLength(); i++)
				{
					if (strcmp(processes[i].title, entry.szExeFile) == 0)
					{
						if (processes[i].type == Process::TYPE_GAME) isGameOn = true;
						found = true;
						break;
					}
				}
				if (!found)
				{
					Process newProcess;
					strcpy(newProcess.title, entry.szExeFile);
					processes.add(newProcess);
				}
				hasEntry = Process32Next(snapshot, &entry);
			}
			CloseHandle(snapshot);
		}

		beginMeasure("Working Hours");
		if (!openTasksNotificationSilenced && isGameOn && amountOfTasksNowWithoutOneShot > 0 && isWorkTime())
		{
			static float timeSinceLastNotify = 10000.0f;
			timeSinceLastNotify += timeSinceLastFrame;
			if (timeSinceLastNotify >= 15 * 60)
			{
				timeSinceLastNotify = 0.0f;
				assetStore::OpenTasks()->play();
			}
		}
	}

	int32_t drawTable(const char* title, const std::function<bool(Task&)>& predicate, bool& contentsChanged, bool showMoveToNow, bool showCountdown, bool showDone, bool showFollowUp, bool highlightRareTasks, bool showAdvancable, bool sorted)
	{
		int32_t amountDrawn = 0;
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), title);
		if (ImGui::BeginTable("table2", 7, ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 400);
			if(showCountdown) ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
			ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("EEE", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("FFF", ImGuiTableColumnFlags_WidthStretch);
			static bbe::List<size_t> indices; // Avoid allocations
			indices.clear();
			for (size_t i = 0; i < tasks.getLength(); i++)
			{
				Task& t = tasks[i];
				if (!predicate(t)) continue;
				indices.add(i);
			}
			if (sorted)
			{
				indices.sort([&](const size_t& a, const size_t& b) 
					{
						return tasks[a].nextPossibleExecution() < tasks[b].nextPossibleExecution();
					});
			}
			for (size_t indexindex = 0; indexindex < indices.getLength(); indexindex++)
			{
				const size_t i = indices[indexindex];
				Task& t = tasks[i];
				if (!predicate(t)) continue;
				amountDrawn++;
				ImGui::PushID(i);
				ImGui::TableNextRow();
				int32_t column = 0;
				ImGui::TableSetColumnIndex(column++);
				if ((highlightRareTasks && t.repeatDays > 1) || t.oneShot)
				{
					const bool poosibleTodoToday = (t.nextPossibleExecution().hasPassed() || t.nextPossibleExecution().isToday());
					if(t.oneShot)              { ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "(!)"); tooltip("A one shot task."); }
					else if(poosibleTodoToday) { ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.8f, 1.0f), "(?)"); tooltip("A rare task that could be done today."); }
					else                       { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(!)"); tooltip("A rare task."); }
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
					modifiedTitle = modifiedTitle.replace("[SEC]", bbe::String::format("%.2d", value));
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
					tooltip(t.nextPossibleExecution().toString());
				}
				ImGui::TableSetColumnIndex(column++);
				if (showDone)
				{
					if (t.inputType == Task::InputType::NONE)
					{
						if (!t.oneShot)
						{
							if (ImGui::Button("Done"))
							{
								t.execDone();
								contentsChanged = true;
							}
						}
						else
						{
							if(securityButton("Done"))
							{
								tasks.removeIndex(i);
								contentsChanged = true;
							}
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
				if (showAdvancable)
				{
					ImGui::TableSetColumnIndex(column++);
					if (t.advanceable && (t.earlyAdvanceable || isLateAdvanceableTime()) && ImGui::Button("Advance"))
					{
						t.execAdvance();
						contentsChanged = true;
					}
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
		taskChanged |= ImGui::Checkbox("Monday", &t.canBeMo);
		ImGui::SameLine(); taskChanged |= ImGui::Checkbox("Tuesday", &t.canBeTu);
		ImGui::SameLine(); taskChanged |= ImGui::Checkbox("Wednesday", &t.canBeWe);
		ImGui::SameLine(); taskChanged |= ImGui::Checkbox("Thursday", &t.canBeTh);
		ImGui::SameLine(); taskChanged |= ImGui::Checkbox("Friday", &t.canBeFr);
		ImGui::SameLine(); taskChanged |= ImGui::Checkbox("Saturday", &t.canBeSa);
		ImGui::SameLine(); taskChanged |= ImGui::Checkbox("Sunday", &t.canBeSu);
		
		taskChanged |= ImGui::Checkbox("Advanceable", &t.advanceable);
		tooltip("Can \"done\" even if it's not planned for today.");
		if (t.advanceable)
		{
			taskChanged |= ImGui::Checkbox("Preparation", &t.preparation);
			tooltip("Will never be shown for the current day. Inteded for Tasks that prepare stuff for tomorrow, e.g. pre brewing some coffee.");

			taskChanged |= ImGui::Checkbox("Early Advanceable", &t.earlyAdvanceable);
			tooltip("If unchecked, the task is only advanceable after 18:00.");
		}
		taskChanged |= ImGui::Checkbox("One Shot", &t.oneShot);
		tooltip("Delets the Task when Done.");
		taskChanged |= ImGui::InputInt("Follow Up  (in Minutes)", &t.followUp);
		tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
		taskChanged |= ImGui::InputInt("Follow Up2 (in Minutes)", &t.followUp2);
		tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
		taskChanged |= ImGui::InputInt("Internal Value", &t.internalValue);
		tooltip("An internal value that can be printed out in the title via %%d, [SEC], and [MIN].");
		taskChanged |= ImGui::InputInt("Internal Value Increase", &t.internalValueIncrease);
		tooltip("Increases the Internal Value on ever Done by this much.");
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

#ifdef BrowserStuff
	bbe::List<bbe::String> getDomains()
	{
		bbe::List<bbe::String> retVal;
		//By: Barmak Shemirani see https://stackoverflow.com/a/48507146/7130273
		//Modified to return a bbe::List<bbe::String>
		//         and only the domain part
		HWND hwnd = NULL;
		while (true)
		{
			hwnd = FindWindowEx(NULL, hwnd, "Chrome_WidgetWin_1", NULL);
			if (!hwnd)
				break;
			if (!IsWindowVisible(hwnd))
				continue;

			CComQIPtr<IUIAutomation> uia;
			if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia)
				continue;

			CComPtr<IUIAutomationElement> root;
			if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root)
				continue;

			CComPtr<IUIAutomationCondition> condition;
			uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
				CComVariant(0xC354), &condition);

			//or use edit control's name instead
			//uia->CreatePropertyCondition(UIA_NamePropertyId, 
			//      CComVariant(L"Address and search bar"), &condition);

			CComPtr<IUIAutomationElement> edit;
			if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &edit))
				|| !edit)
				continue; //maybe we don't have the right tab, continue...

			CComVariant url;
			edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);

			char buffer[1024] = {};
			wcstombs(buffer, url.bstrVal, sizeof(buffer) / sizeof(wchar_t));
			//retVal.add(bbe::String(buffer).split("/")[0]);
			retVal.add(bbe::String(buffer));
		}
		return retVal;
	}
#endif

	bool securityButton(const char* text)
	{
		bool retVal = ImGui::Button(shiftPressed ? text : "[Shift]") && shiftPressed;
		tooltip("Hold shift to activate this button.");
		return retVal;
	}

	void tooltip(const char* text)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
		{
			ImGui::Text(text);
			ImGui::EndTooltip();
		}
	}
	void tooltip(const bbe::String& text)
	{
		tooltip(text.getRaw());
	}

	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
		if (!editMode)
		{
			ImGuiViewport viewport = *ImGui::GetMainViewport();
			viewport.WorkSize.x *= 0.6f;
			ImGui::SetNextWindowPos(viewport.WorkPos);
			ImGui::SetNextWindowSize(viewport.WorkSize);
			ImGui::Begin("MainWindow", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{
				if (ImGui::BeginTabBar("MainWindowTabs")) {
					if (ImGui::BeginTabItem("Tasks")) {
						bool contentsChanged = false;
						drawTable("Now",      [](Task& t) { return t.nextPossibleExecution().hasPassed() && !t.preparation; },                                                    contentsChanged, false, false, true,  true, false, false, false);
						drawTable("Today",    [](Task& t) { return !t.nextPossibleExecution().hasPassed() && t.nextPossibleExecution().isToday(); },                              contentsChanged, true,  true,  true,  true, false, false, false);
						drawTable("Tomorrow", [](Task& t) { return t.isImportantTomorrow(); },                                                                                    contentsChanged, true,  false, false, true, true , true , false);
						drawTable("Later",    [](Task& t) { return !t.nextPossibleExecution().hasPassed() && !t.nextPossibleExecution().isToday() && !t.isImportantTomorrow(); }, contentsChanged, true,  true,  true,  true, false, false, true);
						if (contentsChanged)
						{
							tasks.writeToFile();
						}
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Other Stuff")) {
						ImGui::Text("For future Use");
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
			}
			ImGui::End();

			viewport.WorkPos.x += viewport.WorkSize.x;
			viewport.WorkSize.x *= ImGui::GetMainViewport()->WorkSize.x * 0.4f;
			viewport.WorkSize.y *= 1.f / 3.f;
			ImGui::SetNextWindowPos(viewport.WorkPos);
			ImGui::SetNextWindowSize(viewport.WorkSize);
			ImGui::Begin("Info", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{
				ImGui::Text("Build: " __DATE__ ", " __TIME__);
				bbe::String s = "Night Start in: " + (getNightStart() - bbe::TimePoint()).toString();
				ImGui::Text(s.getRaw());
				ImGui::Checkbox("Silence Open Task Notification Sound", &openTasksNotificationSilenced);
				ImGui::Checkbox("Ignore Night", &ignoreNight);
				ImGui::Checkbox("Show Debug Stuff", &showDebugStuff);
			}
			ImGui::End();

			viewport.WorkPos.y = viewport.WorkSize.y;
			ImGui::SetNextWindowPos(viewport.WorkPos);
			ImGui::SetNextWindowSize(viewport.WorkSize);
			ImGui::Begin("Processes", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{
				static bool showSystem = false;
				ImGui::Checkbox("Show System", &showSystem);
				static bool showOther = false;
				ImGui::SameLine();
				ImGui::Checkbox("Show Other", &showOther);
				static bool showGames = false;
				ImGui::SameLine();
				ImGui::Checkbox("Show Games", &showGames);
				if (ImGui::BeginTable("tableProcesses", 2, ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 600);
					ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
					bool processChanged = false;
					for (size_t i = 0; i < processes.getLength(); i++)
					{
						Process& p = processes[i];
						if (p.type == Process::TYPE_SYSTEM && !showSystem) continue;
						if (p.type == Process::TYPE_OTHER && !showOther) continue;
						if (p.type == Process::TYPE_GAME && !showGames) continue;
						ImGui::PushID(i);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(p.title);

						ImGui::TableSetColumnIndex(1);
						if (ImGui::BeginCombo("Type", p.inputTypeStr))
						{
							for (int i = 0; i < IM_ARRAYSIZE(Process::typeStrings); i++)
							{
								if (ImGui::Selectable(Process::typeStrings[i]))
								{
									p.inputTypeStr = Process::typeStrings[i];
									p.type = i;
									processChanged = true;
								}
							}
							ImGui::EndCombo();
						}
						ImGui::PopID();
					}
					if (processChanged)
					{
						processes.writeToFile();
					}
					ImGui::EndTable();
				}
			}
			ImGui::End();


			viewport.WorkPos.y = viewport.WorkSize.y * 2;
			ImGui::SetNextWindowPos(viewport.WorkPos);
			ImGui::SetNextWindowSize(viewport.WorkSize);
			ImGui::Begin("URLs", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{
#ifdef BrowserStuff
				auto tabNames = getDomains();
				for (size_t i = 0; i < tabNames.getLength(); i++)
				{
					ImGui::Text(tabNames[i].getRaw());
				}
#endif
			}
			ImGui::End();
		}
		else
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::Begin("Edit Mode", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Edit Mode (Hit CTRL+E to leave)");
			ImGui::Separator();
			ImGui::Text("Build: " __DATE__ ", " __TIME__);
			ImGui::Separator();

			{
				static Task tempTask;
				drawEditableTask(tempTask);
				tempTask.sanity();

				static int year = 0;
				static int month = 0;
				static int day = 0;

				if (year == 0 && month == 0 && day == 0)
				{
					bbe::TimePoint now;
					year = now.getYear();
					month = (int)now.getMonth();
					day = now.getDay();
				}

				ImGui::PushItemWidth(100);
				ImGui::Text("First execution: ");
				ImGui::SameLine(); ImGui::InputInt("##year",  &year,  0, 0); tooltip("Year");
				ImGui::SameLine(); ImGui::InputInt("##month", &month, 0, 0); tooltip("Month");
				ImGui::SameLine(); ImGui::InputInt("##day",   &day,   0, 0); tooltip("Day");
				ImGui::PopItemWidth();

				if (year < 2023) year = 2023;
				month = bbe::Math::clamp(month, 1, 12);
				day = bbe::Math::clamp(day, 1, 31); // TODO: Not all months have 31 days... Use Proper Date Picker?

				if (ImGui::Button("New Task"))
				{
					tempTask.inputType = Task::strToInputType(tempTask.inputTypeStr);
					tempTask.setNextExecution(year, month, day);
					tasks.add(tempTask);
					tempTask = Task();
				}
			}
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			static char searchBuffer[128] = {};
			ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::NewLine();

			bool tasksChanged = false;
			size_t deletionIndex = (size_t)-1;
			for (size_t i = 0; i < tasks.getLength(); i++)
			{
				Task& t = tasks[i];
				if (searchBuffer[0] != 0 && !bbe::String(t.title).containsIgnoreCase(searchBuffer)) continue;
				ImGui::PushID(i);
				if (securityButton("Delete Task"))
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
				tasksChanged |= drawEditableTask(t);
				ImGui::Text(t.previousExecution.toString().getRaw()); tooltip("Previous Execution");
				ImGui::Text(t.nextPossibleExecution().toString().getRaw()); tooltip("Next Execution");
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

		if (showDebugStuff)
		{
			ImGui::ShowDemoWindow();
			ImPlot::ShowDemoWindow();
			drawMeasure(brush);
		}
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