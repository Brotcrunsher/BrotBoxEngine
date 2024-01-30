#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <AtlBase.h>
#include <UIAutomation.h>
#include "AssetStore.h"
#include "imgui_internal.h"

//TODO: GATW: Also play "Open Tasks" sound when opening time wasting URLs
//TODO: Butchered looks on non 4k
//TODO: Implement proper date picker
//TODO: Redo
//TODO: Sometimes freezes. I suspect process stuff? track what the longest time of each section was and display somewhere.
//TODO: Countdown beeps when starting and stopping startable tasks
//TODO: Gamification, add a score how much time I needed to do all Now Tasks
//TODO: New feature: Stopwatch ("Pizza done")
//TODO: Bug: When switching headphones, the sound system seems to die.

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
	char title[1024] = {};
	int32_t repeatDays = 0;
	bbe::TimePoint previousExecution = bbe::TimePoint::epoch();
private:
	bbe::TimePoint nextExecution; // Call nextPossibleExecution from the outside! 
public:
	bool canBeSu = true;
	int32_t followUp = 0; // In minutes. When clicking follow up, the task will be rescheduled the same day.
	int32_t internalValue = 0;
	int32_t internalValueIncrease = 0;
	int32_t followUp2 = 0;
	enum /*Non-Class*/ InputType
	{
		IT_NONE,
		IT_INTEGER,
		IT_FLOAT,
	};
	int32_t inputType = IT_NONE;
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
	char clipboard[1024] = {};
	bool lateTimeTask = false;
	enum /*Non-Class*/ DateType
	{
		DT_DYNAMIC = 0,
		DT_YEARLY  = 1,
		// dt_monthly = 2, // Not implemented
	};
	int32_t dateType = DT_DYNAMIC;
	int32_t dtYearlyMonth = 1;
	int32_t dtYearlyDay = 1;
	bool startable = false;
	bbe::TimePoint endWorkTime = bbe::TimePoint::epoch();
	bool indefinitelyAdvanceable = false;

	// Non-Persisted Helper Data below.
	int32_t inputInt = 0;
	float inputFloat = 0;
	mutable bool armedToPlaySoundNewTask = false;
	mutable bool armedToPlaySoundDone = false;

private:
	bool timePointElapsed(const bbe::TimePoint& tp, bool& armed) const
	{
		if (tp.hasPassed())
		{
			if (armed)
			{
				armed = false;
				return true;
			}
		}
		else
		{
			armed = true;
		}
		return false;
	}
public:

	bool shouldPlaySoundNewTask() const
	{
		return timePointElapsed(nextPossibleExecution(), armedToPlaySoundNewTask);
	}

	bool shouldPlaySoundDone() const
	{
		return timePointElapsed(endWorkTime, armedToPlaySoundDone);
	}

	void execDone()
	{
		internalValue += internalValueIncrease;
		previousExecution = bbe::TimePoint();
		if (dateType == DT_DYNAMIC)
		{
			nextExecution = toPossibleTimePoint(previousExecution.nextMorning().plusDays(repeatDays - 1));
		}
		else if (dateType == DT_YEARLY)
		{
			nextExecution = getNextYearlyExecution();
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
		armedToPlaySoundNewTask = false;
		nextExecution = bbe::TimePoint();
	}
	void execAdvance()
	{
		internalValue += internalValueIncrease;
		previousExecution = bbe::TimePoint();
		if (dateType == DT_DYNAMIC)
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
		else if (dateType == DT_YEARLY)
		{
			nextExecution = getNextYearlyExecution();
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
		buffer.write(inputType);
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
		buffer.writeNullString(clipboard);
		buffer.write(lateTimeTask);
		buffer.write(dateType);
		buffer.write(dtYearlyMonth);
		buffer.write(dtYearlyDay);
		buffer.write(startable);
		endWorkTime.serialize(buffer);
		buffer.write(indefinitelyAdvanceable);
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
		buffer.read(retVal.inputType);
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
		strcpy(retVal.clipboard, buffer.readNullString());
		buffer.read(retVal.lateTimeTask, false);
		buffer.read(retVal.dateType);
		buffer.read(retVal.dtYearlyMonth, 1);
		buffer.read(retVal.dtYearlyDay, 1);
		buffer.read(retVal.startable, false);
		retVal.endWorkTime = bbe::TimePoint::deserialize(buffer);
		buffer.read(retVal.indefinitelyAdvanceable, false);

		return retVal;
	}

	void nextExecPlusDays(int32_t days)
	{
		nextExecution = toPossibleTimePoint(nextPossibleExecution().plusDays(days).toMorning(), days > 0);
	}

	bbe::TimePoint nextPossibleExecution() const
	{
		if (!nextExecution.hasPassed()) return nextExecution;
		return toPossibleTimePoint(bbe::TimePoint());
	}

	bool isPossibleWeekday(const bbe::TimePoint& tp) const
	{
		if (!canBeMo && tp.isMonday())    return false;
		if (!canBeTu && tp.isTuesday())   return false;
		if (!canBeWe && tp.isWednesday()) return false;
		if (!canBeTh && tp.isThursday())  return false;
		if (!canBeFr && tp.isFriday())    return false;
		if (!canBeSa && tp.isSaturday())  return false;
		if (!canBeSu && tp.isSunday())    return false;
		return true;
	}

	bbe::TimePoint toPossibleTimePoint(const bbe::TimePoint& tp, bool forwardInTime = true) const
	{
		bbe::TimePoint retVal = tp;
		for (int32_t i = 0; i < 14; i++)
		{
			if (!isPossibleWeekday(retVal))
			{
				if (forwardInTime) retVal = retVal.nextMorning();
				else               retVal = retVal.plusDays(-1);
			}
			else
			{
				break;
			}
		}
		if (preparation && retVal.isToday()) retVal = retVal.nextMorning();
		return retVal;
	}

	bool isImportantTomorrow() const
	{
		//TODO: "Interesting" calculation for "tomorrow"...
		bbe::TimePoint tomorrow = bbe::TimePoint().nextMorning().plusDays(1).plusHours(-6);
		if (!isPossibleWeekday(tomorrow)) return false;
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

	int32_t amountPossibleWeekdays() const
	{
		int32_t retVal = 0;
		if (canBeMo) retVal++;
		if (canBeTu) retVal++;
		if (canBeWe) retVal++;
		if (canBeTh) retVal++;
		if (canBeFr) retVal++;
		if (canBeSa) retVal++;
		if (canBeSu) retVal++;

		return retVal;
	}

	bbe::TimePoint getNextYearlyExecution() const
	{
		bbe::TimePoint now;
		return toPossibleTimePoint(bbe::TimePoint::fromDate(now.getYear() + 1, dtYearlyMonth, dtYearlyDay).nextMorning());
	}

	bool wasDoneToday() const
	{
		return previousExecution.isToday();
	}

	bbe::Duration getWorkDurationLeft() const
	{
		return endWorkTime - bbe::TimePoint();
	}

	bool wasStartedToday() const
	{
		return endWorkTime.isToday();
	}

	void execStart()
	{
		endWorkTime = bbe::TimePoint().plusSeconds(internalValue);
	}
};

struct Process
{
	char title[1024] = {};

	enum /*Non-Class*/ Type
	{
		TYPE_UNKNOWN = 0,
		TYPE_SYSTEM = 1,
		TYPE_OTHER = 2,
		TYPE_GAME = 3,
	};
	int32_t type = TYPE_UNKNOWN;


	// Non-Persisted Helper Data below.

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

		return retVal;
	}
};

struct ClipboardContent
{
	char content[1024] = {};


	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.writeNullString(content);
	}
	static ClipboardContent deserialize(bbe::ByteBufferSpan& buffer)
	{
		ClipboardContent retVal;

		strcpy(retVal.content, buffer.readNullString());

		return retVal;
	}
};

class MyGame : public bbe::Game
{
private:
	bbe::SerializableList<Task> tasks = bbe::SerializableList<Task>("config.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableList<Process> processes = bbe::SerializableList<Process>("processes.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableList<ClipboardContent> clipboardContent = bbe::SerializableList<ClipboardContent>("Clipboard.dat", "ParanoiaConfig", bbe::Undoable::YES);
	bool shiftPressed = false;
	bool isGameOn = false;
	bool openTasksNotificationSilenced = false;
	bool showDebugStuff = false;
	bool ignoreNight = false;
	bool tabSwitchRequestedLeft = false;
	bool tabSwitchRequestedRight = false;
	bool forcePrepare = false;

	bbe::List<HICON> trayIconsRed;
	bbe::List<HICON> trayIconsGreen;
	bbe::List<HICON> trayIconsBlue;
	size_t trayIconIndex = 0;

	int32_t amountOfTasksNow = 0;
	// Madness names
	int32_t amountOfTasksNowWithoutOneShotWithoutLateTime = 0;
	int32_t amountOfTasksNowWithoutOneShotWithLateTime = 0;

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

	bool trayIconVisible() const
	{
		// Quite Hacky! Let me explain...
		// Switching a tray icon is an expensive operation, even when
		// the tray icon isn't even visible. So this function tries to
		// find out if the tray icon is visible or not. If not, we can
		// early out of the tray switching function (see
		// setCurrentTrayIcon). The only way to figure this out that I
		// found was asking for the position of the tray icon. When it's
		// not visible, the left attribute of the Rect returns something
		// around 300.
		// However! Getting the position of the tray icon is itself
		// (maybe surprisingly) an expensive opteration. So this function
		// only checks every second. If this second hasn't passed, then
		// it simply returns false. In a worse case this leads to a very
		// small (1 second) delay before the animation starts.
		// Honestly - this is dumb! But oh well. It kinda works.
		static bbe::TimePoint autoFalseUntil;

		if (!autoFalseUntil.hasPassed())
		{
			return false;
		}

		NOTIFYICONIDENTIFIER identifier = {};
		identifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
		identifier.hWnd = Hwnd;
		identifier.uID = 0;
		memset(&identifier.guidItem, 0, sizeof(identifier.guidItem));
		RECT rect;
		Shell_NotifyIconGetRect(&identifier, &rect);

		bool retVal = rect.left > 500;
		if (!retVal) autoFalseUntil = bbe::TimePoint().plusSeconds(1);
		return retVal;
	}

	void setCurrentTrayIcon(bool firstCall)
	{
		if (!firstCall && !trayIconVisible())
		{
			return;
		}
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
		if (daysSinceQualifyingDate > 120) daysSinceQualifyingDate = 120;

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
		shiftPressed = isKeyDown(bbe::Key::LEFT_SHIFT);
		tabSwitchRequestedLeft  = isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::Q);
		tabSwitchRequestedRight = isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::E);


		beginMeasure("Play Task Sounds");
		if (tasks.getList().any([](const Task& t) { return t.shouldPlaySoundNewTask(); }))
		{
			assetStore::NewTask()->play();
		}
		if (tasks.getList().any([](const Task& t) { return t.shouldPlaySoundDone(); }))
		{
			assetStore::Done()->play();
		}

		if (exitRequested)
		{
			closeWindow();
		}

		beginMeasure("Tray Icon");
		setCurrentTrayIcon(false);

		beginMeasure("Night Time");
		if (isNightTime())
		{
			if (ignoreNight)
			{
				static float timeSinceLastSureSound = 0.0f;
				timeSinceLastSureSound += timeSinceLastFrame;
				if (timeSinceLastSureSound > 3600 /*1 hour*/)
				{
					timeSinceLastSureSound = 0;
					assetStore::AreYouSure()->play();
				}
			}
			else
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
		}
		if (shouldPlayAlmostNightWarning())
		{
			assetStore::AlmostNightTime()->play();
		}

		beginMeasure("Task Amount Calculation");
		amountOfTasksNow = 0;
		amountOfTasksNowWithoutOneShotWithoutLateTime = 0;
		amountOfTasksNowWithoutOneShotWithLateTime = 0;

		for (size_t i = 0; i < tasks.getLength(); i++)
		{
			Task& t = tasks[i];
			if (t.nextPossibleExecution().hasPassed())
			{
				amountOfTasksNow++;
				if (!t.oneShot)
				{
					if (t.lateTimeTask)
					{
						amountOfTasksNowWithoutOneShotWithLateTime++;
					}
					else
					{
						amountOfTasksNowWithoutOneShotWithoutLateTime++;
					}
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
		if (!openTasksNotificationSilenced && isGameOn && 
			(
				   (amountOfTasksNowWithoutOneShotWithoutLateTime > 0 &&  isWorkTime())
				|| (amountOfTasksNowWithoutOneShotWithLateTime    > 0 && !isWorkTime())
			))
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

	int32_t drawTable(const char* title, const std::function<bool(Task&)>& predicate, bool& requiresWrite, bool showMoveToNow, bool showCountdown, bool showDone, bool showFollowUp, bool highlightRareTasks, bool showAdvancable, bool respectIndefinitelyFlag, bool sorted)
	{
		int32_t amountDrawn = 0;
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), title);
		if (ImGui::BeginTable("table2", 7, ImGuiTableFlags_RowBg))
		{
			                    ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 400);
			if (showCountdown)  ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
			                    ImGui::TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthFixed, 100);
			if (showFollowUp)   ImGui::TableSetupColumn("EEE", ImGuiTableColumnFlags_WidthFixed, 100);
			if (showFollowUp)   ImGui::TableSetupColumn("FFF", ImGuiTableColumnFlags_WidthFixed, 100);
			if (showAdvancable) ImGui::TableSetupColumn("GGG", ImGuiTableColumnFlags_WidthFixed, 100);
			if (showMoveToNow)  ImGui::TableSetupColumn("HHH", ImGuiTableColumnFlags_WidthFixed, 175);
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
			size_t deletedTasks = 0;
			for (size_t indexindex = 0; indexindex < indices.getLength(); indexindex++)
			{
				const size_t i = indices[indexindex] - deletedTasks;
				Task& t = tasks[i];
				if (!predicate(t)) continue;
				amountDrawn++;
				ImGui::PushID(i);
				ImGui::TableNextRow();
				if(ImGui::TableGetHoveredRow() == indexindex) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, 0xFF333333);
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
				if (t.clipboard[0] == '\0')
				{
					ImGui::Text(modifiedTitle.getRaw(), t.internalValue);
				}
				else
				{
					if (ClickableText(modifiedTitle.getRaw(), t.internalValue))
					{
						ImGui::SetClipboardText(t.clipboard);
					}
				}
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
					if (t.inputType == Task::IT_NONE)
					{
						bool showDoneButton = true;
						if (t.startable)
						{
							if (!t.wasDoneToday())
							{
								bbe::Duration dur = t.getWorkDurationLeft();
								if (!t.wasStartedToday())
								{
									showDoneButton = false;
									if (ImGui::Button("Start"))
									{
										t.execStart();
										requiresWrite = true;
									}
								}
								else if (!dur.isNegative())
								{
									ImGui::Text(dur.toString().getRaw());
									showDoneButton = false;
								}
							}
						}

						if (showDoneButton)
						{
							if (!t.oneShot)
							{
								if (ImGui::Button("Done"))
								{
									t.execDone();
									requiresWrite = true;
								}
							}
							else
							{
								if (securityButton("Done"))
								{
									tasks.removeIndex(i);
									// Doesn't require write cause removeIndex already does that.
									deletedTasks++;
								}
							}
						}
					}
					else if (t.inputType == Task::IT_INTEGER)
					{
						if (ImGui::InputInt("##input", &t.inputInt, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							t.history.add(t.inputInt);
							t.execDone();
							requiresWrite = true;
						}
					}
					else if (t.inputType == Task::IT_FLOAT)
					{
						if (ImGui::InputFloat("##input", &t.inputFloat, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							t.history.add(t.inputFloat);
							t.execDone();
							requiresWrite = true;
						}
					}
					else
					{
						throw bbe::IllegalStateException();
					}
				}
				if (showFollowUp)
				{
					ImGui::TableSetColumnIndex(column++);
					if (t.followUp > 0 && ImGui::Button((bbe::String("+") + t.followUp + "min").getRaw()))
					{
						t.execFollowUp();
						requiresWrite = true;
					}
					ImGui::TableSetColumnIndex(column++);
					if (t.followUp2 > 0 && ImGui::Button((bbe::String("+") + t.followUp2 + "min").getRaw()))
					{
						t.execFollowUp2();
						requiresWrite = true;
					}
				}
				if (showAdvancable)
				{
					ImGui::TableSetColumnIndex(column++);
					if (t.advanceable && (!respectIndefinitelyFlag || t.indefinitelyAdvanceable) && (t.earlyAdvanceable || isLateAdvanceableTime() || forcePrepare) && ImGui::Button("Advance"))
					{
						t.execAdvance();
						requiresWrite = true;
					}
				}
				if (showMoveToNow)
				{
					ImGui::TableSetColumnIndex(column++);
					if (ImGui::Button("Move to Now"))
					{
						t.execMoveToNow();
						requiresWrite = true;
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

	bool weekdayCheckbox(const char* label, bool* b, int32_t amountOfWeekdays)
	{
		ImGui::BeginDisabled(amountOfWeekdays <= 1 && *b);
		const bool retVal = ImGui::Checkbox(label, b);
		ImGui::EndDisabled();
		return retVal;
	}

	bool drawEditableTask(Task& t)
	{
		bool taskChanged = false;
		taskChanged |= ImGui::InputText("Title", t.title, sizeof(t.title));
		taskChanged |= combo("Date Type", { "Dynamic", "Yearly" }, t.dateType);
		if (t.dateType == Task::DT_DYNAMIC)
		{
			taskChanged |= ImGui::InputInt("Repeat Days", &t.repeatDays);
		}
		else if(t.dateType == Task::DT_YEARLY)
		{
			ImGui::PushItemWidth(100);
			ImGui::Text("Month/Day: ");
			ImGui::SameLine();
			ImGui::InputInt("##dt_yearly_month", &t.dtYearlyMonth, 0, 0);
			tooltip("Month");
			ImGui::SameLine();
			ImGui::InputInt("##dt_yearly_day", &t.dtYearlyDay, 0, 0);
			tooltip("Day");
			ImGui::PopItemWidth();

			t.dtYearlyMonth = bbe::Math::clamp(t.dtYearlyMonth, 1, 12);
			t.dtYearlyDay = bbe::Math::clamp(t.dtYearlyDay, 1, 31); // TODO: Not all months have 31 days... Use Proper Date Picker?
		}
		const int32_t amountOfWeekdays = t.amountPossibleWeekdays();
		taskChanged |= weekdayCheckbox("Monday", &t.canBeMo, amountOfWeekdays);
		ImGui::SameLine(); taskChanged |= weekdayCheckbox("Tuesday",   &t.canBeTu, amountOfWeekdays);
		ImGui::SameLine(); taskChanged |= weekdayCheckbox("Wednesday", &t.canBeWe, amountOfWeekdays);
		ImGui::SameLine(); taskChanged |= weekdayCheckbox("Thursday",  &t.canBeTh, amountOfWeekdays);
		ImGui::SameLine(); taskChanged |= weekdayCheckbox("Friday",    &t.canBeFr, amountOfWeekdays);
		ImGui::SameLine(); taskChanged |= weekdayCheckbox("Saturday",  &t.canBeSa, amountOfWeekdays);
		ImGui::SameLine(); taskChanged |= weekdayCheckbox("Sunday",    &t.canBeSu, amountOfWeekdays);
		
		taskChanged |= ImGui::Checkbox("Advanceable", &t.advanceable);
		tooltip("Can \"done\" even if it's not planned for today.");
		if (t.advanceable)
		{
			ImGui::Indent(15.0f);
			taskChanged |= ImGui::Checkbox("Preparation", &t.preparation);
			tooltip("Will never be shown for the current day. Inteded for Tasks that prepare stuff for tomorrow, e.g. pre brewing some coffee.");

			taskChanged |= ImGui::Checkbox("Early Advanceable", &t.earlyAdvanceable);
			tooltip("If unchecked, the task is only advanceable after 18:00.");

			taskChanged |= ImGui::Checkbox("Indefinitely Advanceable", &t.indefinitelyAdvanceable);
			tooltip("Can be advanced in the \"Later\" table.");
			ImGui::Unindent(15.0f);
		}
		taskChanged |= ImGui::Checkbox("One Shot", &t.oneShot);
		tooltip("Delets the Task when Done.");
		taskChanged |= ImGui::Checkbox("Late Time Task", &t.lateTimeTask);
		tooltip("A late time task triggers the \"Open Tasks\" sound outside of Working Hours instead of during Working Hours.");
		taskChanged |= ImGui::Checkbox("Startable", &t.startable);
		tooltip("Doesn't show \"Done\" immediately, but instead a start button that starts a count down of the length\nof the internal value in seconds. After that time a sound is played and the \"Done\" Button appears.");
		taskChanged |= ImGui::InputInt("Follow Up  (in Minutes)", &t.followUp);
		tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
		taskChanged |= ImGui::InputInt("Follow Up2 (in Minutes)", &t.followUp2);
		tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
		taskChanged |= ImGui::InputInt("Internal Value", &t.internalValue);
		tooltip("An internal value that can be printed out in the title via %%d, [SEC], and [MIN].");
		taskChanged |= ImGui::InputInt("Internal Value Increase", &t.internalValueIncrease);
		tooltip("Increases the Internal Value on ever Done by this much.");
		taskChanged |= combo("Input Type", { "None", "Integer", "Float" }, t.inputType);

		taskChanged |= ImGui::InputText("Clipboard", t.clipboard, sizeof(t.clipboard));
		tooltip("When clicking the task, this will be sent to your clipboard.");

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

	bool combo(const char* label, const bbe::List<bbe::String>& selections, int32_t& selection)
	{
		bool retVal = false;

		if (ImGui::BeginCombo(label, selections[selection].getRaw()))
		{
			for (int32_t i = 0; i < selections.getLength(); i++)
			{
				if (ImGui::Selectable(selections[i].getRaw()))
				{
					selection = i;
					retVal = true;
				}
			}
			ImGui::EndCombo();
		}

		return retVal;
	}

	bool ClickableText(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		int size = vsnprintf(nullptr, 0, fmt, args);
		static bbe::List<char> buffer; // Avoid allocations
		buffer.resizeCapacity(size + 1);
		vsnprintf(buffer.getRaw(), size + 1, fmt, args);
		va_end(args);

		bool dummy = false;
		return ImGui::Selectable(buffer.getRaw(), &dummy);
	}

	void drawTabViewTasks()
	{
		bool requiresWrite = false;
		drawTable("Now",      [](Task& t) { return t.nextPossibleExecution().hasPassed() && !t.preparation; },                                                    requiresWrite, false, false, true,  true,  false, false, false, false);
		drawTable("Today",    [](Task& t) { return !t.nextPossibleExecution().hasPassed() && t.nextPossibleExecution().isToday(); },                              requiresWrite, true,  true,  true,  true,  false, false, false, false);
		drawTable("Tomorrow", [](Task& t) { return t.isImportantTomorrow(); },                                                                                    requiresWrite, true,  false, false, true,  true , true , false, false);
		drawTable("Later",    [](Task& t) { return !t.nextPossibleExecution().hasPassed() && !t.nextPossibleExecution().isToday() && !t.isImportantTomorrow(); }, requiresWrite, true,  true,  true,  false, false, true , true , true );
		if (requiresWrite)
		{
			tasks.writeToFile();
		}
	}

	void drawTabEditTasks()
	{
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
			ImGui::SameLine(); ImGui::InputInt("##year", &year, 0, 0); tooltip("Year");
			ImGui::SameLine(); ImGui::InputInt("##month", &month, 0, 0); tooltip("Month");
			ImGui::SameLine(); ImGui::InputInt("##day", &day, 0, 0); tooltip("Day");
			ImGui::PopItemWidth();

			if (year < 2023) year = 2023;
			month = bbe::Math::clamp(month, 1, 12);
			day = bbe::Math::clamp(day, 1, 31); // TODO: Not all months have 31 days... Use Proper Date Picker?

			if (ImGui::Button("New Task"))
			{
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
			ImGui::SameLine();
			if (ImGui::Button("Move to Now"))
			{
				t.execMoveToNow();
				tasksChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("+1 Day"))
			{
				t.nextExecPlusDays(1);
				tasksChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("-1 Day"))
			{
				t.nextExecPlusDays(-1);
				tasksChanged = true;
			}
			ImGui::Text(t.endWorkTime.toString().getRaw()); tooltip("End Work Time");
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();
			ImGui::PopID();
			t.sanity();
		}
		tasks.removeIndex(deletionIndex);
		if (tasksChanged)
		{
			tasks.writeToFile();
		}
	}

	void drawTabClipboard()
	{
		static ClipboardContent newContent;
		if (ImGui::InputText("New Content", newContent.content, sizeof(newContent.content), ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
		{
			clipboardContent.add(newContent);
			newContent = ClipboardContent();
		}
		size_t deleteIndex = (size_t)-1;
		for (size_t i = 0; i < clipboardContent.getLength(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::Button("Delete"))
			{
				deleteIndex = i;
			}
			ImGui::SameLine();
			if (ImGui::Button(clipboardContent[i].content))
			{
				setClipboard(clipboardContent[i].content);
			}
			ImGui::PopID();
		}
		if (deleteIndex != (size_t)-1)
		{
			clipboardContent.removeIndex(deleteIndex);
		}
	}

	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
		ImGuiViewport viewport = *ImGui::GetMainViewport();
		viewport.WorkSize.x *= 0.6f;
		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(viewport.WorkSize);
		ImGui::Begin("MainWindow", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			struct Tab
			{
				const char* title = "";
				std::function<void()> run;
			};
			static bbe::List<Tab> tabs =
			{
				Tab{"View Tasks", [this]() { drawTabViewTasks(); }},
				Tab{"Edit Tasks", [this]() { drawTabEditTasks(); }},
				Tab{"Clipboard",  [this]() { drawTabClipboard(); }},
			};
			if (ImGui::BeginTabBar("MainWindowTabs")) {
				static size_t previousShownTab = 0;
				size_t desiredShownTab = previousShownTab;
				const bool programaticTabSwitch = tabSwitchRequestedLeft || tabSwitchRequestedRight;
				if (programaticTabSwitch)
				{
					if (tabSwitchRequestedLeft) desiredShownTab--;
					if (desiredShownTab == (size_t)-1) desiredShownTab = tabs.getLength() - 1;
					if (tabSwitchRequestedRight) desiredShownTab++;
					if (desiredShownTab == tabs.getLength()) desiredShownTab = 0;
				}
				for (size_t i = 0; i < tabs.getLength(); i++)
				{
					Tab& t = tabs[i];
					if (ImGui::BeginTabItem(t.title, nullptr, (programaticTabSwitch && i == desiredShownTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
					{
						previousShownTab = i;
						t.run();
						ImGui::EndTabItem();
					}
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
			tooltip(getNightStart().toString().getRaw());

			ImGui::BeginDisabled(!tasks.canUndo());
			if (ImGui::Button("Undo"))
			{
				tasks.undo();
			}
			ImGui::EndDisabled();

			ImGui::Checkbox("Silence Open Task Notification Sound", &openTasksNotificationSilenced);
			ImGui::Checkbox("Ignore Night", &ignoreNight);
			ImGui::Checkbox("Let me prepare", &forcePrepare); tooltip("Make tasks advancable, even before late time happens.");
			ImGui::Checkbox("Show Debug Stuff", &showDebugStuff);
			ImGui::NewLine();
			ImGui::Text(getMeasuresString().getRaw());
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
					if (ImGui::Button("S"))
					{
						processChanged = true;
						p.type = Process::TYPE_SYSTEM;
					}
					tooltip("System");
					ImGui::SameLine();
					if (ImGui::Button("O"))
					{
						processChanged = true;
						p.type = Process::TYPE_OTHER;
					}
					tooltip("Other");
					ImGui::SameLine();
					if (ImGui::Button("G"))
					{
						processChanged = true;
						p.type = Process::TYPE_GAME;
					}
					tooltip("Game");
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

		if (showDebugStuff)
		{
			ImGui::ShowDemoWindow();
			ImPlot::ShowDemoWindow();
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