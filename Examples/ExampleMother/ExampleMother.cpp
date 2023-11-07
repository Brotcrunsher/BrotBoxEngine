#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <Windows.h>

#define WM_SYSICON        (WM_USER + 1)
#define ID_SOME_ID        1002

NOTIFYICONDATA notifyIconData;
HWND Hwnd;
HMENU Hmenu;
HICON hIcon;
char szClassName[] = "MyWindowClassName";
char szTIP[64] = TEXT("M.O.THE.R");
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

class MyGame;
MyGame* myGame;

struct Task
{
	char title[1024] = {};
	int32_t repeatDays = 0;
	bbe::TimePoint previousExecution;
	bbe::TimePoint nextExecution;
	bool canBeSundays = true;
	int32_t followUp = 0; // In minutes. When clicking follow up, the task will be rescheduled the same day.
	int32_t internalValue = 0;
	int32_t internalValueIncrease = 0;

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
	void execMoveToToday()
	{
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

		return retVal;
	}
};

class MyGame : public bbe::Game
{
private:
	bbe::SerializableList<Task> tasks = bbe::SerializableList<Task>("config.dat", bbe::SerializableList<Task>::ParanoiaMode::PARANOIA);
	bool editMode = false;


public:
	MyGame()
	{
		myGame = this;
	}

	virtual void onStart() override
	{
		setWindowCloseMode(bbe::WindowCloseMode::HIDE);
		setTargetFrametime(1.f / 144.f);

		BYTE ANDmaskIcon[32 * 4];
		bbe::Random rand;
		for (int i = 0; i < sizeof(ANDmaskIcon); i++)
		{
			ANDmaskIcon[i] = rand.randomInt(256);
		}
		BYTE XORmaskIcon[32 * 4];
		memset(XORmaskIcon, 255, sizeof(XORmaskIcon));

		hIcon = CreateIcon(0,    // application instance  
			32,              // icon width 
			32,              // icon height 
			1,               // number of XOR planes 
			1,               // number of bits per pixel 
			ANDmaskIcon,     // AND bitmask  
			XORmaskIcon);    // XOR bitmask 

		WNDCLASSEX wincl;        /* Data structure for the windowclass */
		wincl.hInstance = GetModuleHandle(0);
		wincl.lpszClassName = szClassName;
		wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
		wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
		wincl.cbSize = sizeof(WNDCLASSEX);

		/* Use default icon and mouse-pointer */
		wincl.hIcon = hIcon;
		wincl.hIconSm = hIcon;
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


		memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

		notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = Hwnd;
		notifyIconData.uID = 0;
		notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
		notifyIconData.hIcon = hIcon;
		memcpy_s(notifyIconData.szTip, sizeof(notifyIconData.szTip), szTIP, sizeof(szTIP));


		Shell_NotifyIcon(NIM_ADD, &notifyIconData);
		//Hmenu = CreatePopupMenu();
		//AppendMenu(Hmenu, MF_STRING, ID_SOME_ID, TEXT("Lunch Break!"));
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::E)) editMode = !editMode;
	}

	void drawTable(const std::function<bool(Task&)>& predicate, bool& contentsChanged, bool showMoveToToday)
	{
		if (ImGui::BeginTable("table2", 4))
		{
			ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 400);
			ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthStretch);
			for (size_t i = 0; i < tasks.getLength(); i++)
			{
				Task& t = tasks[i];
				if (!predicate(t)) continue;
				ImGui::PushID(i);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(t.title, t.internalValue);
				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button("Done"))
				{
					t.execDone();
					contentsChanged = true;
				}
				ImGui::TableSetColumnIndex(2);
				if (t.followUp > 0 && ImGui::Button("Follow Up"))
				{
					t.execFollowUp();
					contentsChanged = true;
				}
				if (showMoveToToday)
				{
					ImGui::TableSetColumnIndex(3);
					if (ImGui::Button("Move to Today"))
					{
						t.execMoveToToday();
						contentsChanged = true;
					}
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}

	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
		if (!editMode)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::Begin("Edit Mode", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
			bool contentsChanged = false;
			drawTable([](Task& t) { return t.nextExecution.hasPassed(); }, contentsChanged, false);
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			drawTable([](Task& t) { return !t.nextExecution.hasPassed() && t.nextExecution.isToday(); }, contentsChanged, true);
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			drawTable([](Task& t) { return !t.nextExecution.hasPassed() && !t.nextExecution.isToday(); }, contentsChanged, true);
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			if (contentsChanged)
			{
				tasks.writeToFile();
			}
			ImGui::End();
		}
		else
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::Begin("Edit Mode", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Edit Mode (Hit CTRL+E to leave)");

			{
				static Task tempTask;
				ImGui::InputText("Title", tempTask.title, sizeof(tempTask.title));
				ImGui::InputInt("Repeat Days", &tempTask.repeatDays);
				ImGui::Checkbox("Can be Sundays", &tempTask.canBeSundays);
				ImGui::InputInt("Follow Up (in Minutes)", &tempTask.followUp);
				ImGui::InputInt("Internal Value", &tempTask.internalValue);
				ImGui::InputInt("Internal Value Increase", &tempTask.internalValueIncrease);
				tempTask.sanity();

				if (ImGui::Button("New Task"))
				{
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
				if (ImGui::Button("Delete Task"))
				{
					deletionIndex = i;
				}
				Task& t = tasks[i];
				tasksChanged |= ImGui::InputText("Title", t.title, sizeof(t.title));
				tasksChanged |= ImGui::InputInt("Repeat Days", &t.repeatDays);
				tasksChanged |= ImGui::Checkbox("Can be Sundays", &t.canBeSundays);
				tasksChanged |= ImGui::InputInt("Follow Up (in Minutes)", &t.followUp);
				tasksChanged |= ImGui::InputInt("Internal Value", &t.internalValue);
				tasksChanged |= ImGui::InputInt("Internal Value Increase", &t.internalValueIncrease);
				ImGui::Text(t.previousExecution.toString().getRaw());
				ImGui::Text(t.nextExecution.toString().getRaw());
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
			//if (clicked == ID_SOME_ID)
			//{
			//	std::cout << "Hai!" << std::endl;
			//	myGame->showWindow();
			//}
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