#include "BBE/BrotBoxEngine.h"
#include <iostream>
#define NOMINMAX
#include <Windows.h>
#include <tlhelp32.h>
#include <AtlBase.h>
#include <UIAutomation.h>
#include "AssetStore.h"
#include "imgui_internal.h"
#include "GlobalKeyboard.h"

//TODO: Redo
//TODO: Countdown beeps when starting and stopping startable tasks
//TODO: Gamification, add a score how much time I needed to do all Now Tasks
//TODO: Clipboard: It should be possible to add a title
//TODO: Bug: Crashed when closing Chrome? Only happened once, not easily reproducable.
//TODO: Bug: When switching headphones, the sound system doesn't switch as well. It stays playing sounds on the old device.
//TODO: This file is getting massive. Split?
//TODO: Add console

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
	bool shouldPlayNotificationSounds = true;

	// Non-Persisted Helper Data below.
	int32_t inputInt = 0;
	float inputFloat = 0;
	mutable bool armedToPlaySoundNewTask = false;
	mutable bool armedToPlaySoundDone = false;
	bbe::TimePoint execPointBuffer;
	bbe::TimePoint yearlyBuffer;

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
		buffer.write(shouldPlayNotificationSounds);
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
		buffer.read(retVal.shouldPlayNotificationSounds, true);

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

	void setNextExecution(const bbe::TimePoint& tp)
	{
		setNextExecution(tp.getYear(), (int32_t)tp.getMonth(), tp.getDay());
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

struct Url
{
	char url[1024] = {};
	enum /*Non-Class*/ Type
	{
		TYPE_UNKNOWN = 0,
		TYPE_TIME_WASTER = 1,
		TYPE_WORK = 2,
	};
	int32_t type = TYPE_UNKNOWN;


	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.writeNullString(url);
		buffer.write(type);
	}
	static Url deserialize(bbe::ByteBufferSpan& buffer)
	{
		Url retVal;

		strcpy(retVal.url, buffer.readNullString());
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

struct GeneralConfig
{
	char updatePath[1024] = {};
	int32_t beepEvery = 0;
	char backupPath[1024] = {};

	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.writeNullString(updatePath);
		buffer.write(beepEvery);
		buffer.writeNullString(backupPath);
	}
	static GeneralConfig deserialize(bbe::ByteBufferSpan& buffer)
	{
		GeneralConfig retVal;

		strcpy(retVal.updatePath, buffer.readNullString());
		buffer.read(retVal.beepEvery);
		strcpy(retVal.backupPath, buffer.readNullString());

		return retVal;
	}
};

struct KeyboardTracker
{
	uint32_t keyPressed[(size_t)bbe::Key::LAST + 1] = {};

	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		for (size_t i = 0; i < (size_t)bbe::Key::LAST + 1; i++)
		{
			buffer.write(keyPressed[i]);
		}
	}
	static KeyboardTracker deserialize(bbe::ByteBufferSpan& buffer)
	{
		KeyboardTracker retVal;
		for (size_t i = 0; i < (size_t)bbe::Key::LAST + 1; i++)
		{
			buffer.read(retVal.keyPressed[i]);
		}
		return retVal;
	}
};

struct BrainTeaserScore
{
	int32_t score = 0;
	bbe::TimePoint didItOn;

	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.write(score);
		didItOn.serialize(buffer);
	}
	static BrainTeaserScore deserialize(bbe::ByteBufferSpan& buffer)
	{
		BrainTeaserScore retVal;

		buffer.read(retVal.score);
		retVal.didItOn.deserialize(buffer);

		return retVal;
	}
};

struct Stopwatch
{
	char title[1024] = {};
	int32_t seconds = 0;
	bbe::TimePoint doneAt;

	// Non-Persisted Helper Data below.
	mutable bool soundArmed = false;

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.writeNullString(title);
		buffer.write(seconds);
		doneAt.serialize(buffer);
	}
	static Stopwatch deserialize(bbe::ByteBufferSpan& buffer)
	{
		Stopwatch retVal;

		strcpy(retVal.title, buffer.readNullString());
		buffer.read(retVal.seconds);
		retVal.doneAt.deserialize(buffer);

		return retVal;
	}

	bool shouldPlaySound() const
	{
		if (doneAt.hasPassed())
		{
			if (soundArmed)
			{
				soundArmed = false;
				return true;
			}
		}
		else
		{
			soundArmed = true;
		}
		return false;
	}
};

class MyGame : public bbe::Game
{
private:
	bbe::SerializableList<Task> tasks                           = bbe::SerializableList<Task>             ("config.dat",              "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableList<Process> processes                    = bbe::SerializableList<Process>          ("processes.dat",           "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableList<Url> urls                             = bbe::SerializableList<Url>              ("urls.dat",                "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableList<ClipboardContent> clipboardContent    = bbe::SerializableList<ClipboardContent> ("Clipboard.dat",           "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableObject<GeneralConfig> generalConfig        = bbe::SerializableObject<GeneralConfig>  ("generalConfig.dat",       "ParanoiaConfig");
	bbe::SerializableList<BrainTeaserScore> brainTeaserMemory   = bbe::SerializableList<BrainTeaserScore> ("brainTeaserMemory.dat",   "ParanoiaConfig");
	bbe::SerializableList<BrainTeaserScore> brainTeaserAlphabet = bbe::SerializableList<BrainTeaserScore> ("brainTeaserAlphabet.dat", "ParanoiaConfig");
	bbe::SerializableList<BrainTeaserScore> brainTeaserAdd      = bbe::SerializableList<BrainTeaserScore> ("brainTeaserAdd.dat",      "ParanoiaConfig");
	bbe::SerializableList<Stopwatch> stopwatches                = bbe::SerializableList<Stopwatch>        ("stopwatches.dat",         "ParanoiaConfig");
	bbe::SerializableObject<KeyboardTracker> keyboardTracker    = bbe::SerializableObject<KeyboardTracker>("keyboardTracker.dat"); // No ParanoiaConfig to avoid accidentally logging passwords.

	bool isGameOn = false;
	bool openTasksNotificationSilencedProcess = false;
	bool openTasksNotificationSilencedUrl     = false;
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

	bbe::Random rand;

	bbe::List<float> mousePositions;

	bbe::GlobalKeyboard globalKeyboard;


	bool terriActive = false;

public:
	HICON createTrayIcon(DWORD offset, int redGreenBlue)
	{
		// See: https://learn.microsoft.com/en-us/windows/win32/menurc/using-cursors#creating-a-cursor
		constexpr size_t iconWidth = 32;
		constexpr size_t iconHeight = 32;
		constexpr size_t centerX = iconWidth / 2;
		constexpr size_t centerY = iconHeight / 2;

		bbe::Image image(iconWidth, iconHeight);
		for (size_t x = 0; x < iconWidth; x++)
		{
			for (size_t y = 0; y < iconHeight; y++)
			{
				const DWORD xDiff = x - centerX;
				const DWORD yDiff = y - centerY;
				const DWORD distSq = xDiff * xDiff + yDiff * yDiff;
				const DWORD dist = bbe::Math::sqrt(distSq * 1000) + offset;
				const DWORD cVal = dist % 512;

				const float highlight = (cVal > 255 ? 255 : cVal) / 255.f;
				const float white =     (cVal > 255 ? (cVal - 255) : 0) / 255.f;

				bbe::Color c;
				c.a = 1.0f;
				c.r = white;
				c.g = white;
				c.b = white;

				if (redGreenBlue == 0) /*Red*/
				{
					c.r = highlight;
				}
				else if (redGreenBlue == 1) /*Green*/
				{
					c.g = highlight;
				}
				else if (redGreenBlue == 2) /*Blue*/
				{
					c.b = highlight;
				}
				else
				{
					throw bbe::IllegalArgumentException();
				}

				image.setPixel(x, y, c);
			}
		}

		return image.toIcon();
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

	void exitCallback()
	{
		closeWindow();
	}

	virtual void onStart() override
	{
		setWindowCloseMode(bbe::WindowCloseMode::HIDE);

		createTrayIcons();
		bbe::TrayIcon::init(this, "M.O.THE.R " __DATE__ ", " __TIME__, getCurrentTrayIcon());
		bbe::TrayIcon::addPopupItem("Exit", [&]() { exitCallback(); });

		bbe::backup::setBackupPath(generalConfig->backupPath);
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
		beginMeasure("Mouse Tracking");
		{
			EVERY_MILLISECONDS(100)
			{
				bbe::Vector2 glob = getMouseGlobal();
				mousePositions.add(glob.x);
				mousePositions.add(glob.y);
				if (mousePositions.getLength() >= 20000)
				{
					time_t t;
					time(&t);
					bbe::simpleFile::createDirectory("mousePositions");
					bbe::simpleFile::writeFloatArrToFile(bbe::String("mousePositions/mouse") + t + ".dat", mousePositions);
					mousePositions.clear();
				}
			}
		}

		beginMeasure("Basic Controls");
		setTargetFrametime((isFocused() || isHovered() || terriActive) ? (1.f / 144.f) : (1.f / 10.f));
		tabSwitchRequestedLeft  = isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::Q);
		tabSwitchRequestedRight = isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::E);

		beginMeasure("GlobalKeyboard");
		globalKeyboard.update();
		for (size_t i = 0; i < (size_t)bbe::Key::LAST + 1; i++)
		{
			if (globalKeyboard.isKeyPressed((bbe::Key)i, false))
			{
				keyboardTracker->keyPressed[i]++;
				static int writes = 0;
				writes++;
				if (writes >= 1024)
				{
					// Only writing out every 1024 chars. This way we avoid accidentally
					// writing out passwords to the file (or at least easily reconstructable
					// passwords). Additionally it reduced the amount of IO.
					keyboardTracker.writeToFile();
					writes = 0;
				}
			}
		}

		beginMeasure("Play Task Sounds");
		if (tasks.getList().any([](const Task& t) { return t.shouldPlaySoundNewTask(); }))
		{
			assetStore::NewTask()->play();
		}
		if (tasks.getList().any([](const Task& t) { return t.shouldPlaySoundDone(); }))
		{
			assetStore::Done()->play();
		}
		if (stopwatches.getList().any([](const Stopwatch& sw) { return sw.shouldPlaySound(); }))
		{
			assetStore::Stopwatch()->play();
		}

		beginMeasure("Tray Icon");
		if(bbe::TrayIcon::isVisible()) bbe::TrayIcon::setIcon(getCurrentTrayIcon());

		beginMeasure("Night Time");
		if (isNightTime())
		{
			if (ignoreNight)
			{
				EVERY_HOURS(1)
				{
					assetStore::AreYouSure()->play();
				}
			}
			else
			{
				EVERY_MINUTES(1)
				{
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
				if (!t.oneShot && t.shouldPlayNotificationSounds)
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
		EVERY_SECONDS(10)
		{
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

		beginMeasure("URL Stuff");
		bool timeWasterUrlFound = false;
		EVERY_SECONDS(1)
		{
			auto tabNames = getDomains();
			for (size_t i = 0; i < tabNames.getLength(); i++)
			{
				bool found = false;
				for (size_t k = 0; k < urls.getLength(); k++)
				{
					if (tabNames[i] == urls[k].url)
					{
						found = true;
						if (urls[k].type == Url::TYPE_TIME_WASTER)
						{
							timeWasterUrlFound = true;
						}
						break;
					}
				}
				if (!found)
				{
					Url newUrl;
					strcpy(newUrl.url, tabNames[i].getRaw());
					urls.add(newUrl);
				}
			}
		}

		beginMeasure("Beeper");
		if (generalConfig->beepEvery > 0)
		{
			EVERY_MINUTES(generalConfig->beepEvery)
			{
				assetStore::Beep()->play();
			}
		}

		beginMeasure("Working Hours");
		bool workTodo =
			   (amountOfTasksNowWithoutOneShotWithoutLateTime > 0 &&  isWorkTime())
			|| (amountOfTasksNowWithoutOneShotWithLateTime    > 0 && !isWorkTime());

		if (workTodo)
		{
			// Because Process...
			bool shouldPlayOpenTasks = !openTasksNotificationSilencedProcess && isGameOn;

			// ... because urls.
			shouldPlayOpenTasks |= !openTasksNotificationSilencedUrl && timeWasterUrlFound;
			if (shouldPlayOpenTasks)
			{
				EVERY_MINUTES(15)
				{
					assetStore::OpenTasks()->play();
				}
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
					if(t.oneShot)              { ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "(!)"); ImGui::bbe::tooltip("A one shot task."); }
					else if(poosibleTodoToday) { ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.8f, 1.0f), "(?)"); ImGui::bbe::tooltip("A rare task that could be done today."); }
					else                       { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(!)"); ImGui::bbe::tooltip("A rare task."); }
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
					if (ImGui::bbe::clickableText(modifiedTitle.getRaw(), t.internalValue))
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
					ImGui::bbe::tooltip(t.nextPossibleExecution().toString());
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
								if (ImGui::bbe::securityButton("Done"))
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
		taskChanged |= ImGui::bbe::combo("Date Type", { "Dynamic", "Yearly" }, t.dateType);
		if (t.dateType == Task::DT_DYNAMIC)
		{
			taskChanged |= ImGui::InputInt("Repeat Days", &t.repeatDays);
		}
		else if(t.dateType == Task::DT_YEARLY)
		{
			ImGui::Text("Month/Day: ");
			ImGui::SameLine();
			ImGui::bbe::datePicker("Yearly Pick", &t.yearlyBuffer);
			// TODO: It's possible to change the year in the date picker, which is kinda dumb
			//       for a yearly task. The year is discarded, but the GUI could be nicer.

			t.dtYearlyMonth = (int32_t)t.yearlyBuffer.getMonth();
			t.dtYearlyDay = t.yearlyBuffer.getDay();
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
		ImGui::bbe::tooltip("Can \"done\" even if it's not planned for today.");
		if (t.advanceable)
		{
			ImGui::Indent(15.0f);
			taskChanged |= ImGui::Checkbox("Preparation", &t.preparation);
			ImGui::bbe::tooltip("Will never be shown for the current day. Inteded for Tasks that prepare stuff for tomorrow, e.g. pre brewing some coffee.");

			taskChanged |= ImGui::Checkbox("Early Advanceable", &t.earlyAdvanceable);
			ImGui::bbe::tooltip("If unchecked, the task is only advanceable after 18:00.");

			taskChanged |= ImGui::Checkbox("Indefinitely Advanceable", &t.indefinitelyAdvanceable);
			ImGui::bbe::tooltip("Can be advanced in the \"Later\" table.");
			ImGui::Unindent(15.0f);
		}
		taskChanged |= ImGui::Checkbox("One Shot", &t.oneShot);
		ImGui::bbe::tooltip("Delets the Task when Done.");
		taskChanged |= ImGui::Checkbox("Late Time Task", &t.lateTimeTask);
		ImGui::bbe::tooltip("A late time task triggers the \"Open Tasks\" sound outside of Working Hours instead of during Working Hours.");
		taskChanged |= ImGui::Checkbox("Startable", &t.startable);
		ImGui::bbe::tooltip("Doesn't show \"Done\" immediately, but instead a start button that starts a count down of the length\nof the internal value in seconds. After that time a sound is played and the \"Done\" Button appears.");
		taskChanged |= ImGui::Checkbox("Play Notifications", &t.shouldPlayNotificationSounds);
		ImGui::bbe::tooltip("If set, playing a notification sound when time wasters are open and the task isn't done. Else, play no sound.");
		taskChanged |= ImGui::InputInt("Follow Up  (in Minutes)", &t.followUp);
		ImGui::bbe::tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
		taskChanged |= ImGui::InputInt("Follow Up2 (in Minutes)", &t.followUp2);
		ImGui::bbe::tooltip("Pushes the Task by this many minutes into the future. Useful for Tasks that can be fulfilled multiple times per day.");
		taskChanged |= ImGui::InputInt("Internal Value", &t.internalValue);
		ImGui::bbe::tooltip("An internal value that can be printed out in the title via %%d, [SEC], and [MIN].");
		taskChanged |= ImGui::InputInt("Internal Value Increase", &t.internalValueIncrease);
		ImGui::bbe::tooltip("Increases the Internal Value on ever Done by this much.");
		taskChanged |= ImGui::bbe::combo("Input Type", { "None", "Integer", "Float" }, t.inputType);

		taskChanged |= ImGui::InputText("Clipboard", t.clipboard, sizeof(t.clipboard));
		ImGui::bbe::tooltip("When clicking the task, this will be sent to your clipboard.");

		return taskChanged;
	}

	bbe::List<bbe::String> getDomains()
	{
		bbe::List<bbe::String> retVal;
		//Inspired By: Barmak Shemirani see https://stackoverflow.com/a/48507146/7130273
		//Modified to return a bbe::List<bbe::String>
		//         only the domain part
		//         performance optimized (caching etc.)

		static bool iniDone = false;
		static CComQIPtr<IUIAutomation> uia;
		static CComPtr<IUIAutomationCondition> condition;
		if (!iniDone)
		{
			iniDone = true;
			if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia)
				return retVal;

			//uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
			//	CComVariant(0xC354), &condition);
			// TODO: Localization. We should have something like the above 2 lines, but unfortunately
			//       the layout of chrome seems to have changed quite a bit and some random UI Elements
			//       sometimes fulfill the condition.
			uia->CreatePropertyCondition(UIA_NamePropertyId, 
			      CComVariant(L"Adress- und Suchleiste"), &condition);
		}

		static bool redGreen = true;
		redGreen = !redGreen;
		struct Edit
		{
			CComPtr<IUIAutomationElement> edit;
			bool currentRedGreen = redGreen;
		};
		static std::map<HWND, Edit> editsCache;
		{
			HWND hwnd = NULL;
			while (true)
			{
				hwnd = FindWindowEx(NULL, hwnd, "Chrome_WidgetWin_1", NULL);
				if (!hwnd)
					break;
				if (!IsWindowVisible(hwnd))
					continue;
				if (GetWindowTextLength(hwnd) == 0)
					continue;

				if (editsCache.count(hwnd) != 0)
				{
					editsCache[hwnd].currentRedGreen = redGreen;
					continue;
				}

				CComPtr<IUIAutomationElement> root;
				if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root)
					continue;

				CComPtr<IUIAutomationElement> edit;
				if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &edit))
					|| !edit)
					continue;
				// ^^^^--- This is the actual reason why we do this cache stuff!
				//         Highly expensive operation to call FindFirst.
				editsCache[hwnd] = { edit, redGreen };
			}
		}

		// Remove cache entries that weren't "touched" in this call.
		for (auto it = editsCache.cbegin(); it != editsCache.cend(); )
		{
			if (it->second.currentRedGreen != redGreen)
			{
				editsCache.erase(it++);
			}
			else
			{
				++it;
			}
		}

		for(auto it = editsCache.begin(); it != editsCache.end(); it++)
		{
			CComPtr<IUIAutomationElement> edit = it->second.edit;
			
			CComVariant focus;
			edit->GetCurrentPropertyValue(UIA_HasKeyboardFocusPropertyId, &focus);

			if (focus.boolVal)
				continue; // We do not want to get the tab url if it's currently getting modified

			CComVariant url;
			edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
			
			bbe::String newElem = bbe::String(url.bstrVal).split("/")[0];
			if (   newElem.getLength() > 0
				&& newElem.contains("."))
			{
				retVal.add(newElem);
			}
		}
		return retVal;
	}

	bbe::Vector2 drawTabViewTasks()
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
		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabEditTasks()
	{
		{
			static Task tempTask;
			drawEditableTask(tempTask);
			tempTask.sanity();

			static bbe::TimePoint firstExec;

			ImGui::Text("First execution: ");
			ImGui::SameLine();
			ImGui::bbe::datePicker("First Exec", &firstExec);

			if (ImGui::Button("New Task"))
			{
				tempTask.setNextExecution(firstExec);
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
			if (ImGui::bbe::securityButton("Delete Task"))
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
			tasksChanged |= ImGui::bbe::datePicker("previousExe", &t.previousExecution); ImGui::bbe::tooltip("Previous Execution");
			t.execPointBuffer = t.nextPossibleExecution();
			const bool execPointChanged = ImGui::bbe::datePicker("nextExe",     &t.execPointBuffer); ImGui::bbe::tooltip("Next Execution");
			if (execPointChanged)
			{
				t.setNextExecution(t.execPointBuffer);
				tasksChanged = true;
			}
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
			tasksChanged |= ImGui::bbe::datePicker("EndWork", &t.endWorkTime); ImGui::bbe::tooltip("End Work Time");
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
		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabClipboard()
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
			ImGui::bbe::tooltip(bbe::String(clipboardContent[i].content).hardBreakEvery(100));
			ImGui::PopID();
		}
		if (deleteIndex != (size_t)-1)
		{
			clipboardContent.removeIndex(deleteIndex);
		}
		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabConsole()
	{
		const auto& log = bbe::logging::getLog();
		for (size_t i = 0; i < log.getLength(); i++)
		{
			ImGui::Text(log[i].getRaw());
		}
		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabConfig()
	{
		bool generalConfigChanged = false;
		generalConfigChanged |= ImGui::InputText("Update Path", generalConfig->updatePath, sizeof(generalConfig->updatePath));
		generalConfigChanged |= ImGui::InputInt("Beep every (mins)", &generalConfig->beepEvery);
		if (ImGui::InputText("Backup Path", generalConfig->backupPath, sizeof(generalConfig->backupPath), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			generalConfigChanged = true;
			bbe::backup::setBackupPath(generalConfig->backupPath);
		}

		if (generalConfigChanged)
		{
			generalConfig.writeToFile();
		}
		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabBrainTeasers(bbe::PrimitiveBrush2D& brush)
	{
		static bbe::List<Tab> tabs =
		{
			Tab{"Digit Memory", [&]() { return drawTabBrainTeaserDigitMemory(brush); }},
			Tab{"Alphabet"    , [&]() { return drawTabBrainTeaserAlphabet(brush);    }},
			Tab{"Add"         , [&]() { return drawTabBrainTeaserAdd(brush);         }},
		};
		return drawTabs(tabs);
	}

	void newAddPair(int32_t& left, int32_t& right)
	{
		left = rand.randomInt(10000);
		right = rand.randomInt(10000);
	}

	bbe::Vector2 drawTabBrainTeaserAdd(bbe::PrimitiveBrush2D& brush)
	{
		enum class BTState
		{
			startable,
			playing,
			endscreen,

			invalid
		};
		static BTState state = BTState::startable;
		static bbe::TimePoint nextStateAt;
		static int32_t currentScore = 0;
		static bool freshlyEnteredState = false;
		static bbe::String reason;

		static int32_t left = 0;
		static int32_t right = 0;

		BTState nextState = BTState::invalid;
		static char inputBuf[1024] = {};
		bbe::Vector2 size(1.0f);

		if (state == BTState::startable)
		{
			if (ImGui::Button("Start") || isKeyPressed(bbe::Key::SPACE))
			{
				nextState = BTState::playing;
			}
		}
		else if (state == BTState::playing)
		{
			size.y = 0.1f;
			static auto font = bbe::Font("Arial.ttf", 50);
			brush.fillText(50, 150, bbe::String(left) + " + " + right, font);
			if (nextStateAt.hasPassed())
			{
				nextState = BTState::endscreen;
				reason = "Timeup";
			}


			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("Your answer", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				const int32_t answer = bbe::String(inputBuf).toLong();
				const int32_t correct = left + right;
				if (answer == correct)
				{
					currentScore++;
					newAddPair(left, right);
					memset(inputBuf, 0, sizeof(inputBuf));
				}
				else
				{
					nextState = BTState::endscreen;
					reason = "Wrong Answer ";
					reason += left;
					reason += " + ";
					reason += right;
					reason += " = ";
					reason += correct;
					reason += " But your answer was: ";
					reason += answer;
				}
			}
		}
		else if (state == BTState::endscreen)
		{
			ImGui::Text("Score: %d", currentScore);
			ImGui::Text("%s", reason.getRaw());
			if (ImGui::Button("Start over") || isKeyPressed(bbe::Key::SPACE))
			{
				nextState = BTState::startable;
			}
		}
		else
		{
			throw std::runtime_error("Wrong state");
		}

		if (nextState != BTState::invalid)
		{
			freshlyEnteredState = true;
			state = nextState;
			if (nextState == BTState::startable)
			{
				currentScore = 0;
			}
			else if (nextState == BTState::playing)
			{
				newAddPair(left, right);
				nextStateAt = bbe::TimePoint().plusSeconds(60);
			}
			else if (nextState == BTState::endscreen)
			{
				BrainTeaserScore bts;
				bts.score = currentScore;
				bts.didItOn = bbe::TimePoint();
				brainTeaserAdd.add(bts);
			}
			else
			{
				throw std::runtime_error("Illegal State");
			}
		}
		else
		{
			freshlyEnteredState = false;
		}
		return size;
	}

	void newAlphabetPair(char& left, char& right)
	{
		left = 'A' + rand.randomInt(26);
		do
		{
			right = 'A' + rand.randomInt(26);
		} while (left == right);
	}

	bbe::Vector2 drawTabBrainTeaserAlphabet(bbe::PrimitiveBrush2D& brush)
	{
		enum class BTState
		{
			startable,
			playing,
			endscreen,

			invalid
		};
		static BTState state = BTState::startable;
		static bbe::TimePoint nextStateAt;
		static int32_t currentScore = 0;
		static bool freshlyEnteredState = false;
		static bbe::String reason;

		static char leftChar = 'A';
		static char rightChar = 'A';

		BTState nextState = BTState::invalid;
		bbe::Vector2 size(1.0f);

		if (state == BTState::startable)
		{
			if (ImGui::Button("Start") || isKeyPressed(bbe::Key::SPACE))
			{
				nextState = BTState::playing;
			}
		}
		else if (state == BTState::playing)
		{
			static auto font = bbe::Font("Arial.ttf", 50);
			brush.fillChar(50, 100, leftChar, font);
			brush.fillChar(150, 100, rightChar, font);
			size.y = 0.f;
			if (nextStateAt.hasPassed())
			{
				nextState = BTState::endscreen;
				reason = "Timeup";
			}
			enum class good
			{
				dunno,
				yes,
				no
			};
			good g = good::dunno;
			if (isKeyPressed(bbe::Key::LEFT) || isKeyPressed(bbe::Key::A))
			{
				if (leftChar < rightChar) g = good::yes;
				else g = good::no;
			}
			if (isKeyPressed(bbe::Key::RIGHT) || isKeyPressed(bbe::Key::D))
			{
				if (rightChar < leftChar) g = good::yes;
				else g = good::no;
			}
			if (g == good::yes)
			{
				currentScore++;
				newAlphabetPair(leftChar, rightChar);
			}
			else if (g == good::no)
			{
				nextState = BTState::endscreen;
				reason = "Wrong selection";
				reason += leftChar;
				reason += rightChar;
			}
		}
		else if (state == BTState::endscreen)
		{
			ImGui::Text("Score: %d", currentScore);
			ImGui::Text("%s", reason.getRaw());
			if (ImGui::Button("Start over") || isKeyPressed(bbe::Key::SPACE))
			{
				nextState = BTState::startable;
			}
		}
		else
		{
			throw std::runtime_error("Wrong state");
		}

		if (nextState != BTState::invalid)
		{
			freshlyEnteredState = true;
			state = nextState;
			if (nextState == BTState::startable)
			{
				currentScore = 0;
			}
			else if (nextState == BTState::playing)
			{
				newAlphabetPair(leftChar, rightChar);
				nextStateAt = bbe::TimePoint().plusSeconds(60);
			}
			else if (nextState == BTState::endscreen)
			{
				BrainTeaserScore bts;
				bts.score = currentScore;
				bts.didItOn = bbe::TimePoint();
				brainTeaserAlphabet.add(bts);
			}
			else
			{
				throw std::runtime_error("Illegal State");
			}
		}
		else
		{
			freshlyEnteredState = false;
		}
		return size;
	}

	bbe::Vector2 drawTabBrainTeaserDigitMemory(bbe::PrimitiveBrush2D& brush)
	{
		enum class BTState
		{
			startable,
			showing,
			waiting,
			entering,
			endscreen,

			invalid,
		};
		static BTState state = BTState::startable;
		static bbe::TimePoint nextStateAt;
		constexpr int32_t startScore = 3;
		static int32_t currentScore = startScore;
		static char patternBuf[1024] = {};
		static char inputBuf[sizeof(patternBuf)] = {};
		static bool freshlyEnteredState = false;

		BTState nextState = BTState::invalid;
		bbe::Vector2 size(1.0f);

		if (state == BTState::startable)
		{
			if (ImGui::Button("Start"))
			{
				nextState = BTState::showing;
			}
		}
		else if (state == BTState::showing)
		{
			static auto font = bbe::Font("Arial.ttf", 50);
			brush.fillText(50, 100, patternBuf, font);
			size.y = 0.f;
			if (nextStateAt.hasPassed())
			{
				nextState = BTState::waiting;
			}
		}
		else if (state == BTState::waiting)
		{
			ImGui::Text((nextStateAt - bbe::TimePoint()).toString().getRaw());
			if (nextStateAt.hasPassed())
			{
				nextState = BTState::entering;
			}
		}
		else if (state == BTState::entering)
		{
			if(freshlyEnteredState) ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("Your answer", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (bbe::String(inputBuf) == bbe::String(patternBuf))
				{
					// Woho!
					nextState = BTState::showing;
				}
				else
				{
					// Oh noes! :(
					nextState = BTState::endscreen;
				}
			}
		}
		else if (state == BTState::endscreen)
		{
			ImGui::Text("Input:  %s", inputBuf);
			ImGui::Text("Actual: %s", patternBuf);
			if (ImGui::Button("Start over"))
			{
				nextState = BTState::startable;
			}
		}
		else
		{
			throw std::runtime_error("Wrong state");
		}

		if (nextState != BTState::invalid)
		{
			freshlyEnteredState = true;
			state = nextState;
			if (nextState == BTState::startable)
			{
				currentScore = startScore;
			}
			else if (nextState == BTState::showing)
			{
				nextStateAt = bbe::TimePoint().plusSeconds(10);
				currentScore++;
				for (int32_t i = 0; i < currentScore; i++)
				{
					int32_t r = rand.randomInt(10);
					patternBuf[i] = '0' + r;
				}
				patternBuf[currentScore] = '\0';
				memset(inputBuf, 0, sizeof(inputBuf));
			}
			else if (nextState == BTState::waiting)
			{
				nextStateAt = bbe::TimePoint().plusSeconds(10);
			}
			else if (nextState == BTState::entering)
			{
				// Nothing to do
			}
			else if (nextState == BTState::endscreen)
			{
				BrainTeaserScore bts;
				bts.score = currentScore;
				bts.didItOn = bbe::TimePoint();
				brainTeaserMemory.add(bts);
			}
			else
			{
				throw std::runtime_error("Illegal State");
			}
		}
		else
		{
			freshlyEnteredState = false;
		}
		return size;
	}

	struct Tab
	{
		const char* title = "";
		std::function<bbe::Vector2()> run;
	};

	bbe::Vector2 drawTabs(const bbe::List<Tab>& tabs, size_t* previousShownTab = nullptr, bool switchLeft = false, bool switchRight = false)
	{
		bbe::Vector2 sizeMult(1.0f, 1.0f);
		if (ImGui::BeginTabBar("MainWindowTabs")) {
			size_t desiredShownTab = 0;
			bool programaticTabSwitch = false;
			if (previousShownTab)
			{
				desiredShownTab = *previousShownTab;
				programaticTabSwitch = switchLeft || switchRight;
				if (programaticTabSwitch)
				{
					if (switchLeft) desiredShownTab--;
					if (desiredShownTab == (size_t)-1) desiredShownTab = tabs.getLength() - 1;
					if (switchRight) desiredShownTab++;
					if (desiredShownTab == tabs.getLength()) desiredShownTab = 0;
				}
			}
			for (size_t i = 0; i < tabs.getLength(); i++)
			{
				const Tab& t = tabs[i];
				if (ImGui::BeginTabItem(t.title, nullptr, (programaticTabSwitch && i == desiredShownTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
				{
					if(previousShownTab) *previousShownTab = i;
					sizeMult = t.run();
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
		return sizeMult;
	}

	bbe::Vector2 drawTabStopwatch()
	{
		{
			static Stopwatch newStopwatch;
			ImGui::InputText("Title", newStopwatch.title, sizeof(newStopwatch.title));
			ImGui::InputInt("Seconds", &newStopwatch.seconds);
			if (ImGui::Button("Create"))
			{
				stopwatches.add(newStopwatch);
				newStopwatch = Stopwatch();
			}
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();

			bool swChanged = false;
			size_t deleteIndex = -1;
			if (ImGui::BeginTable("table", 7, ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed, 100);
				for (size_t i = 0; i < stopwatches.getLength(); i++)
				{
					ImGui::PushID(i);
					Stopwatch& sw = stopwatches[i];

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					if (!sw.doneAt.hasPassed())
					{
						bbe::String s = (sw.doneAt - bbe::TimePoint()).toString();
						ImGui::Text("%s", s.getRaw());
					}
					ImGui::TableNextColumn();
					if (ImGui::Button(sw.title))
					{
						swChanged = true;
						sw.doneAt = bbe::TimePoint().plusSeconds(sw.seconds);
					}
					ImGui::TableNextColumn();
					ImGui::Text("%d", sw.seconds);

					ImGui::TableNextColumn();
					if (ImGui::Button("Delete"))
					{
						deleteIndex = i;
					}

					ImGui::PopID();
				}
				ImGui::EndTable();
			}

			if (swChanged)
			{
				stopwatches.writeToFile();
			}
			if (deleteIndex != (size_t)-1)
			{
				stopwatches.removeIndex(deleteIndex);
			}
		}

		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabMouseTracking(bbe::PrimitiveBrush2D& brush)
	{
		static bbe::Image image;
		static bool loaded = false;
		if (ImGui::Button("Do it!"))
		{
			bbe::List<bbe::Vector2> positions;
			float maxX = 0;
			float maxY = 0;
			float minX = 0;
			float minY = 0;
			bbe::simpleFile::forEachFile("mousePositions", [&](const bbe::String& path) {
				bbe::List<float> contents = bbe::simpleFile::readFloatArrFromFile(path);
				for (size_t i = 0; i < contents.getLength(); i += 2)
				{
					float& x = contents[i + 0];
					float& y = contents[i + 1];
					positions.add({ x, y });
					maxX = bbe::Math::max(maxX, x);
					maxY = bbe::Math::max(maxY, y);
					minX = bbe::Math::min(minX, x);
					minY = bbe::Math::min(minY, y);
				}
				});

			bbe::Grid<float> grid(maxX - minX + 1, maxY - minY + 1);
			float maxValue = 0;
			for (size_t i = 0; i < positions.getLength(); i++)
			{
				bbe::Vector2& p = positions[i];
				grid[p.x][p.y]++;
				maxValue = bbe::Math::max(maxValue, grid[p.x][p.y]);
			}

			image = bbe::Image(maxX - minX + 1, maxY - minY + 1);
			loaded = true;
			for (size_t x = 0; x < grid.getWidth(); x++)
			{
				for (size_t y = 0; y < grid.getHeight(); y++)
				{
					//grid[x][y] /= maxValue;
					image.setPixel(x, y, bbe::Color(grid[x][y] > 0 ? 1.f : 0.f));
				}
			}
		}

		if(loaded) brush.drawImage(0, 200, 800, 400, image);
		return bbe::Vector2(1.0f, 0.1f);
	}

	bbe::Vector2 drawTabKeyboardTracking(bbe::PrimitiveBrush2D& brush)
	{
		static bool normalize = true;
		ImGui::Checkbox("Normalize", &normalize);

		using K = bbe::Key;
		struct DrawnKey
		{
			K key;
			bbe::Vector2 pos;
			float value;
		};
		bbe::List<DrawnKey> keys = {
			{K::_1, {-0.3f,-1}},{K::_2, {0.7f,-1}},{K::_3, {1.7f,-1}},{K::_4, {2.7f,-1}},{K::_5, {3.7f,-1}},{K::_6, {4.7f,-1}},{K::_7, {5.7f,-1}},{K::_8, {6.7f,-1}},{K::_9, {7.7f,-1}},{K::_0, {8.7f,-1}},
			{K::Q, {0.0f, 0}},{K::W, {1.0f, 0}},{K::E, {2.0f, 0}},{K::R, {3.0f, 0}},{K::T, {4.0f, 0}},{K::Z, {5.0f, 0}},{K::U, {6.0f, 0}},{K::I, {7.0f, 0}},{K::O, {8.0f, 0}},{K::P, {9.0f, 0}},
			{K::A, {0.3f, 1}},{K::S, {1.3f, 1}},{K::D, {2.3f, 1}},{K::F, {3.3f, 1}},{K::G, {4.3f, 1}},{K::H, {5.3f, 1}},{K::J, {6.3f, 1}},{K::K, {7.3f, 1}},{K::L, {8.3f, 1}},
			{K::Y, {0.6f, 2}},{K::X, {1.6f, 2}},{K::C, {2.6f, 2}},{K::V, {3.6f, 2}},{K::B, {4.6f, 2}},{K::N, {5.6f, 2}},{K::M, {6.6f, 2}}
		};

		float min = 10000000000.f;
		float max = 0.0f;
		for (size_t i = 0; i < keys.getLength(); i++)
		{
			DrawnKey& k = keys[i];
			k.value = keyboardTracker->keyPressed[(size_t)k.key];
			min = bbe::Math::min(min, k.value);
			max = bbe::Math::max(max, k.value);
		}

		if (!normalize) min = 0.0f;

		for (size_t i = 0; i < keys.getLength(); i++)
		{
			DrawnKey& k = keys[i];
			k.value = (k.value - min) / (max - min);
			brush.setColorRGB(bbe::Color(k.value, k.value, k.value));
			brush.fillText(30 + k.pos.x * 60, 400 + k.pos.y * 60, bbe::keyCodeToString(k.key), 40);
		}

		return bbe::Vector2(1.0f, 0.2f);
	}

	bbe::Vector2 drawTabTerri(bbe::PrimitiveBrush2D& brush)
	{
		ImGui::Checkbox("Active", &terriActive);
		if (globalKeyboard.isKeyPressed(bbe::Key::Q))
		{
			terriActive = !terriActive;
		}
		if (terriActive)
		{
			bbe::Image screenshot = bbe::Image::screenshot(1832, 192, 1, 71);

			int i;
			for (i = 0; i < 8; i++)
			{
				bbe::Color c = screenshot.getPixel(0, 70 - 10 * i);
				float val = (c.r + c.g + c.b) / 3.0f;
				if (val < 0.5f) break;
			}
			static int previousI = 0;
			if (previousI != i)
			{
				if (i == 0) assetStore::One()->play();
				if (i == 1) assetStore::Two()->play();
				if (i == 2) assetStore::Three()->play();
				if (i == 3) assetStore::Four()->play();
				if (i == 4) assetStore::Five()->play();
				if (i == 5) assetStore::Six()->play();
				if (i == 6) assetStore::Seven()->play();
				if (i == 7) assetStore::Eight()->play();
				previousI = i;
			}
		}
		return bbe::Vector2(1.0f);
	}

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		static bbe::Vector2 sizeMult(1.0f, 1.0f);
		ImGuiViewport viewport = *ImGui::GetMainViewport();
		viewport.WorkSize.x *= 0.6f * sizeMult.x;
		viewport.WorkSize.y *= sizeMult.y;
		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(viewport.WorkSize);
		ImGui::Begin("MainWindow", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			static bbe::List<Tab> tabs =
			{
				Tab{"View Tasks", [&]() { return drawTabViewTasks();          }},
				Tab{"Edit Tasks", [&]() { return drawTabEditTasks();          }},
				Tab{"Clipboard",  [&]() { return drawTabClipboard();          }},
				Tab{"Brain-T",    [&]() { return drawTabBrainTeasers(brush);  }},
				Tab{"Stopwatch",  [&]() { return drawTabStopwatch();          }},
				Tab{"MouseTrack", [&]() { return drawTabMouseTracking(brush); }},
				Tab{"KybrdTrack", [&]() { return drawTabKeyboardTracking(brush); }},
				Tab{"Terri",      [&]() { return drawTabTerri(brush);         }},
				Tab{"Console",    [&]() { return drawTabConsole();            }},
				Tab{"Config",     [&]() { return drawTabConfig();             }},
			};
			static size_t previousShownTab = 0;
			sizeMult = drawTabs(tabs, &previousShownTab, tabSwitchRequestedLeft, tabSwitchRequestedRight);
		}
		ImGui::End();

		viewport = *ImGui::GetMainViewport();
		viewport.WorkSize.x *= 0.6f;
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
			ImGui::bbe::tooltip(getNightStart().toString().getRaw());

			ImGui::BeginDisabled(!tasks.canUndo());
			if (ImGui::Button("Undo"))
			{
				tasks.undo();
			}
			ImGui::EndDisabled();
			const static bbe::String desiredName = bbe::simpleFile::getAutoStartDirectory() + "ExampleMother.exe.lnk";
			static bool exists = bbe::simpleFile::doesFileExist(desiredName); // Avoid doing IO every frame.
			ImGui::BeginDisabled(exists);
			if (ImGui::Button("Add to Autostart"))
			{
				bbe::simpleFile::createLink(desiredName, bbe::simpleFile::getExecutablePath(), bbe::simpleFile::getWorkingDirectory());
				exists = true;
			}
			ImGui::EndDisabled();

			static bool updatePathExists = false;
			if (!updatePathExists)
			{
				// Avoiding multiple IO calls.
				EVERY_SECONDS(10)
				{
					updatePathExists = bbe::simpleFile::doesFileExist(generalConfig->updatePath);
				}
			}
			if (updatePathExists)
			{
				if (ImGui::Button("Update"))
				{
					bbe::String batchFileName = "update.bat";
					bbe::simpleFile::deleteFile(batchFileName);

					{
						std::ofstream file;
						file.open(batchFileName.getRaw(), std::ios::out);

						file << "taskkill /f /im ExampleMother.exe\n";
						file << "xcopy /s /y \"" + bbe::String(generalConfig->updatePath) + "\" " + bbe::simpleFile::getExecutablePath() + "\n";
						file << "start ExampleMother.exe\n";
						file << "del update.bat\n";
					}

					bbe::simpleFile::executeBatchFile("update.bat");
				}
			}

			ImGui::Checkbox("Silence Open Task Notification Sound (Process)", &openTasksNotificationSilencedProcess);
			ImGui::Checkbox("Silence Open Task Notification Sound (Url)",     &openTasksNotificationSilencedUrl);
			ImGui::Checkbox("Ignore Night", &ignoreNight);
			ImGui::Checkbox("Let me prepare", &forcePrepare); ImGui::bbe::tooltip("Make tasks advancable, even before late time happens.");
			ImGui::Checkbox("Show Debug Stuff", &showDebugStuff);
			ImGui::NewLine();
			ImGui::Text("Playing sounds: %d", (int)getAmountOfPlayingSounds());
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
					ImGui::bbe::tooltip("System");
					ImGui::SameLine();
					if (ImGui::Button("O"))
					{
						processChanged = true;
						p.type = Process::TYPE_OTHER;
					}
					ImGui::bbe::tooltip("Other");
					ImGui::SameLine();
					if (ImGui::Button("G"))
					{
						processChanged = true;
						p.type = Process::TYPE_GAME;
					}
					ImGui::bbe::tooltip("Game");
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
			static bool showTimeWasters = false;
			ImGui::Checkbox("Show Time Wasters", &showTimeWasters);
			static bool showWork = false;
			ImGui::SameLine();
			ImGui::Checkbox("Show Work", &showWork);
			if (ImGui::BeginTable("tableUrls", 2, ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 600);
				ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
				bool urlChanged = false;
				size_t deletionIndex = (size_t)-1;
				for (size_t i = 0; i < urls.getLength(); i++)
				{
					Url& url = urls[i];
					if (url.type == Url::TYPE_TIME_WASTER && !showTimeWasters) continue;
					if (url.type == Url::TYPE_WORK        && !showWork) continue;
					ImGui::PushID(i);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(url.url);

					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button("Delete"))
					{
						deletionIndex = i;
					}
					ImGui::SameLine();
					if (ImGui::Button("T"))
					{
						urlChanged = true;
						url.type = Url::TYPE_TIME_WASTER;
					}
					ImGui::bbe::tooltip("Time Waster");
					ImGui::SameLine();
					if (ImGui::Button("W"))
					{
						urlChanged = true;
						url.type = Url::TYPE_WORK;
					}
					ImGui::bbe::tooltip("Work");
					ImGui::PopID();
				}
				if (urlChanged)
				{
					urls.writeToFile();
				}
				urls.removeIndex(deletionIndex);
				ImGui::EndTable();
			}
		}
		ImGui::End();

		if (showDebugStuff)
		{
			ImGui::ShowDemoWindow();
			ImPlot::ShowDemoWindow();
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	HWND hWnd = GetConsoleWindow(); 
	FreeConsole();
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "M.O.THE.R - Memory of the repetitive");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}