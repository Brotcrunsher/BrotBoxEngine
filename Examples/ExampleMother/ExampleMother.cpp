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
#include "Task.h"

//TODO: Redo
//TODO: Countdown beeps when starting and stopping startable tasks
//TODO: Gamification, add a score how much time I needed to do all Now Tasks
//TODO: Clipboard: It should be possible to add a title
//TODO: Bug: Crashed when closing Chrome? Only happened once, not easily reproducable.
//TODO: Bug: When switching headphones, the sound system doesn't switch as well. It stays playing sounds on the old device.
//TODO: This file is getting massive. Split?
//TODO: bbe::String should have bbe::List<char>
//TODO: New Feature: Lists! e.g. for lists of movies to watch etc.
			// It follows

struct Process
{
	bbe::String title;

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
		buffer.write(title);
		buffer.write(type);
	}
	static Process deserialize(bbe::ByteBufferSpan& buffer)
	{
		Process retVal;

		buffer.read(retVal.title);
		buffer.read(retVal.type);

		return retVal;
	}
};

struct Url
{
	bbe::String url;
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
		buffer.write(url);
		buffer.write(type);
	}
	static Url deserialize(bbe::ByteBufferSpan& buffer)
	{
		Url retVal;

		buffer.read(retVal.url);
		buffer.read(retVal.type);

		return retVal;
	}
};

struct ClipboardContent
{
	bbe::String content;


	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.write(content);
	}
	static ClipboardContent deserialize(bbe::ByteBufferSpan& buffer)
	{
		ClipboardContent retVal;

		buffer.read(retVal.content);

		return retVal;
	}
};

struct GeneralConfig
{
	bbe::String updatePath;
	int32_t beepEvery = 0;
	bbe::String backupPath;

	// Non-Persisted Helper Data below.

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.write(updatePath);
		buffer.write(beepEvery);
		buffer.write(backupPath);
	}
	static GeneralConfig deserialize(bbe::ByteBufferSpan& buffer)
	{
		GeneralConfig retVal;

		buffer.read(retVal.updatePath);
		buffer.read(retVal.beepEvery);
		buffer.read(retVal.backupPath);

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
		buffer.write(didItOn);
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
	bbe::String title;
	int32_t seconds = 0;
	bbe::TimePoint doneAt;

	// Non-Persisted Helper Data below.
	mutable bool soundArmed = false;

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.write(title);
		buffer.write(seconds);
		buffer.write(doneAt);
	}
	static Stopwatch deserialize(bbe::ByteBufferSpan& buffer)
	{
		Stopwatch retVal;

		buffer.read(retVal.title);
		buffer.read(retVal.seconds);
		buffer.read(retVal.doneAt);

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
	SubsystemTask tasks;

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

	bbe::List<HICON> trayIconsRed;
	bbe::List<HICON> trayIconsGreen;
	bbe::List<HICON> trayIconsBlue;
	size_t trayIconIndex = 0;

	bbe::Random rand;

	bbe::List<float> mousePositions;

	bbe::GlobalKeyboard globalKeyboard;

	bool nextBrainTeaser = false;
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
		if (tasks.hasCurrentTask()) return trayIconsBlue;
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
		tasks.update();
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
					if (processes[i].title == entry.szExeFile)
					{
						if (processes[i].type == Process::TYPE_GAME) isGameOn = true;
						found = true;
						break;
					}
				}
				if (!found)
				{
					Process newProcess;
					newProcess.title = entry.szExeFile;
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
					newUrl.url = tabNames[i].getRaw();
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
		if (tasks.hasPotentialTaskComplaint())
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

	bbe::Vector2 drawTabClipboard()
	{
		static ClipboardContent newContent;
		if (ImGui::bbe::InputText("New Content", newContent.content, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
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
		generalConfigChanged |= ImGui::bbe::InputText("Update Path", generalConfig->updatePath);
		generalConfigChanged |= ImGui::InputInt("Beep every (mins)", &generalConfig->beepEvery);
		if (ImGui::bbe::InputText("Backup Path", generalConfig->backupPath, ImGuiInputTextFlags_EnterReturnsTrue))
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
		static size_t previouslyShown = 0;
		bool dummy = false;
		auto retVal = drawTabs(tabs, &previouslyShown, dummy, nextBrainTeaser);
		return retVal;
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
			if (ImGui::Button("Start") || isKeyPressed(bbe::Key::SPACE) || isKeyPressed(bbe::Key::ENTER))
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
			if (ImGui::Button("Start over") || isKeyPressed(bbe::Key::SPACE) || isKeyPressed(bbe::Key::ENTER))
			{
				nextState = BTState::startable;
				nextBrainTeaser = 2;
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
			if (ImGui::Button("Start") || isKeyPressed(bbe::Key::SPACE) || isKeyPressed(bbe::Key::ENTER))
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
			if (ImGui::Button("Start over") || isKeyPressed(bbe::Key::SPACE) || isKeyPressed(bbe::Key::ENTER))
			{
				nextState = BTState::startable;
				nextBrainTeaser = 2;
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
		static bbe::String patternBuf;
		static bbe::String inputBuf;
		static bool freshlyEnteredState = false;

		BTState nextState = BTState::invalid;
		bbe::Vector2 size(1.0f);

		if (state == BTState::startable)
		{
			if (ImGui::Button("Start") || isKeyPressed(bbe::Key::SPACE) || isKeyPressed(bbe::Key::ENTER))
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
			if (ImGui::bbe::InputText("Your answer", inputBuf, ImGuiInputTextFlags_EnterReturnsTrue))
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
			ImGui::Text("Input:  %s", inputBuf.getRaw());
			ImGui::Text("Actual: %s", patternBuf.getRaw());
			if (ImGui::Button("Start over") || isKeyPressed(bbe::Key::SPACE) || isKeyPressed(bbe::Key::ENTER))
			{
				nextState = BTState::startable;
				nextBrainTeaser = 2;
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
				patternBuf = "";
				currentScore = startScore;
			}
			else if (nextState == BTState::showing)
			{
				nextStateAt = bbe::TimePoint().plusSeconds(10);
				currentScore++;
				while(patternBuf.getLength() < currentScore)
				{
					int32_t r = rand.randomInt(10);
					patternBuf += r;
				}
				inputBuf = "";
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

	bbe::Vector2 drawTabs(const bbe::List<Tab>& tabs, size_t* previousShownTab, bool &switchLeft, bool &switchRight)
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
				switchLeft = false;
				switchRight = false;
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
			ImGui::bbe::InputText("Title", newStopwatch.title);
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
				Tab{"View Tasks", [&]() { return tasks.drawTabViewTasks();    }},
				Tab{"Edit Tasks", [&]() { return tasks.drawTabEditTasks();    }},
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

			tasks.drawUndoButton();

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
			static bool unlockCrashButton = false;
			ImGui::Checkbox("Unlock Crash Button", &unlockCrashButton);
			ImGui::SameLine();
			ImGui::BeginDisabled(!unlockCrashButton);
			if (ImGui::Button("Crash!"))
			{
				*((volatile int*)0);
			}
			ImGui::EndDisabled();

			ImGui::Checkbox("Silence Open Task Notification Sound (Process)", &openTasksNotificationSilencedProcess);
			ImGui::Checkbox("Silence Open Task Notification Sound (Url)",     &openTasksNotificationSilencedUrl);
			ImGui::Checkbox("Ignore Night", &ignoreNight);
			ImGui::Checkbox("Let me prepare", &tasks.forcePrepare); ImGui::bbe::tooltip("Make tasks advancable, even before late time happens.");
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