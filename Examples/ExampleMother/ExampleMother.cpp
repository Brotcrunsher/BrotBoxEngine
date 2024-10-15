#include "tinyxml2.h"
#include "BBE/BrotBoxEngine.h"
#include "BBE/SimpleProcess.h"
#include "BBE/AdafruitMacroPadRP2040.h"
#include <iostream>
#include <cmath>
#define NOMINMAX
// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>
#include <tlhelp32.h>
#include <AtlBase.h>
#include <UIAutomation.h>
#include "AssetStore.h"
#include "imgui_internal.h"
#include "GlobalKeyboard.h"
#include "EMTask.h"
#include "EMProcess.h"
#include "EMUrl.h"
#include "EMTab.h"
#include "EMBrainTeaser.h"
#include "BBE/ChatGPTComm.h"
#include "BBE/Microphone.h"

//TODO: If openal is multithreaded, then why don't we launch static sounds on the main thread and push the info over to the audio thread for later processing?
//      Careful when doing this ^^^^^^ - Audio Restart on device change?
//TODO: Time selector (next to date picker)
//TODO: Serializable List/Object should somehow handle versions... it's really complicated to do that within the nice BBE_SERIALIZABLE_DATA macro though.
//TODO: Ada functionality: Kill all timewasting programs (Risky? "Oopsie I pressed the kill button... arghs")
//TODO: Nighttime configurable
//TODO: Latetime configurable
//TODO: Non eearly tasks should be greyed out during early hours
//TODO: Left a contingent Task running (oopsie). A fail safe of some kind would be nice. Some kind of warning system?
//TODO: Ada functionality: Open a webbrowser and URL bla
//TODO: Google Calendar link (finally learn OAuth 2 properly, not just basics...)
//TODO: The "Elevate" button is really kinda unsecure. It would be much better if we instead do the firewall modification in a separate process that is short lived and terminates quickly. Less of a security vulnerability then.

//TODO: Show average driving time


struct ClipboardContent
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), content),
		((int32_t), adaKey, 0)
	)
};

struct StreakDay
{
	BBE_SERIALIZABLE_DATA(
		((bbe::TimePoint), day)
	)
};

struct GeneralConfig
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), updatePath),
		((int32_t), beepEvery),
		((bbe::String), backupPath),
		((bbe::String), serverKeyFilePath),
		((bbe::String), serverAddress),
		((int32_t), serverPort, 3490),
		((float), baseMonitorBrightness1, 1.0f),
		((float), baseMonitorBrightness2, 1.0f),
		((float), baseMonitorBrightness3, 1.0f),
		((bool), windowSet, false),
		((int32_t), windowPosX, 0),
		((int32_t), windowPosY, 0),
		((int32_t), windowSizeX, 0),
		((int32_t), windowSizeY, 0),
		((bool), windowMaximized)
	)
};

struct KeyboardTracker
{
	BBE_SERIALIZABLE_DATA(
		((std::array<uint32_t, (size_t)bbe::Key::LAST + 1>), keyPressed)
	)
};

struct SeenServerTaskId
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), id)
	)

		auto operator<=>(const SeenServerTaskId&) const = default;
};

struct Stopwatch
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), title),
		((int32_t), seconds),
		((bbe::TimePoint), doneAt)
	)
		mutable bool soundArmed = false;

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

struct RememberList
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), title),
		((bbe::List<bbe::String>), entries)
	)

		// Non-Persisted Helper Data below.
		bbe::String newEntryBuffer;
};

struct ChatGPTConfig
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), apiKey)
	)
};

struct WeaterConfig
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), city)
	)
};

struct NewsEntry
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), title),
		((bbe::String), description),
		((bbe::String), link)
	)
	bool wasRead = false;

	bool isNull() const
	{
		return title       == ""
			&& description == ""
			&& link        == "";
	}

	bool operator==(const NewsEntry& other) const
	{
		return title       == other.title
			&& description == other.description
			&& link        == other.link;
	}
};

struct NewsConfig
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), url),
		((bbe::String), iterationPath),
		((bbe::String), titlePath),
		((bbe::String), descriptionPath),
		((bbe::String), linkPath)
	)

	// Not persisted.
	std::shared_future<bbe::simpleUrlRequest::UrlRequestResult> queryFuture;
	bbe::String queryData;
	bbe::List<NewsEntry> newsEntries;
};

enum class IconCategory
{
	NONE,
	RED,
	GREEN,
	BLUE,
};

BOOL CALLBACK MinimizeWindowCallback(HWND hwnd, LPARAM lParam) {
	char className[256];
	GetClassName(hwnd, className, sizeof(className));

	if (strcmp(className, "Shell_TrayWnd") != 0 && strcmp(className, "Shell_SecondaryTrayWnd") != 0 && hwnd != (HWND)lParam && IsWindowVisible(hwnd)) {
		BBELOGLN("Minimizing: " << className);
		ShowWindow(hwnd, SW_MINIMIZE);
	}
	else
	{
		BBELOGLN("NOT Minimizing: " << className);
	}
	return TRUE;
}

class MyGame : public bbe::Game
{
private:
	SubsystemTask tasks;
	SubsystemProcess processes;
	SubsystemUrl urls;
	SubsystemBrainTeaser brainTeasers = SubsystemBrainTeaser(this);

	bbe::SerializableList<ClipboardContent> clipboardContent    = bbe::SerializableList<ClipboardContent> ("Clipboard.dat",           "ParanoiaConfig", bbe::Undoable::YES);
	bbe::SerializableObject<GeneralConfig> generalConfig        = bbe::SerializableObject<GeneralConfig>  ("generalConfig.dat",       "ParanoiaConfig");
	bbe::SerializableList<Stopwatch> stopwatches                = bbe::SerializableList<Stopwatch>        ("stopwatches.dat",         "ParanoiaConfig");
	bbe::SerializableList<RememberList> rememberLists           = bbe::SerializableList<RememberList>     ("RememberLists.dat",       "ParanoiaConfig");
	bbe::SerializableList<StreakDay> streakDays                 = bbe::SerializableList<StreakDay>        ("streakDays.dat",          "ParanoiaConfig");
	bbe::SerializableObject<KeyboardTracker> keyboardTracker    = bbe::SerializableObject<KeyboardTracker>("keyboardTracker.dat"); // No ParanoiaConfig to avoid accidentally logging passwords.
	bbe::SerializableList<SeenServerTaskId> seenServerTaskIds   = bbe::SerializableList<SeenServerTaskId> ("SeenServerTaskIds.dat",   "ParanoiaConfig");
	bbe::SerializableObject<ChatGPTConfig> chatGPTConfig        = bbe::SerializableObject<ChatGPTConfig>  ("ChatGPTConfig.dat",       "ParanoiaConfig");
	bbe::SerializableObject<WeaterConfig> weatherConfig         = bbe::SerializableObject<WeaterConfig>   ("WeaterConfig.dat",        "ParanoiaConfig");
	bbe::SerializableList<NewsConfig> newsConfig                = bbe::SerializableList<NewsConfig>       ("NewsConfig.dat",          "ParanoiaConfig");
	bbe::SerializableList<NewsEntry> readNews                   = bbe::SerializableList<NewsEntry>        ("ReadNews.dat",            "ParanoiaConfig");

	bbe::ChatGPTComm chatGPTComm; // ChatGPT communication object
	std::future<bbe::ChatGPTQueryResponse> chatGPTFuture; // Future for async ChatGPT queries
	std::future<bbe::Sound> chatGPTTTSFuture; // Future for TTS
	bbe::Sound currentTTSSound;
	bool ttsSoundSet = false;

	bool openTasksSilenced = false;
	bool openTasksSilencedIndefinitely = false;
	bbe::TimePoint openTasksSilencedEnd = bbe::TimePoint::epoch();
	bool showDebugStuff = false;
	bool ignoreNight = false;
	bool tabSwitchRequestedLeft = false;
	bool tabSwitchRequestedRight = false;

	bbe::List<HICON> trayIconsRed;
	bbe::List<HICON> trayIconsGreen;
	bbe::List<HICON> trayIconsBlue;
	size_t trayIconIndex = 0;

	bbe::List<float> mousePositions;

	bbe::GlobalKeyboard globalKeyboard;
	bool keyboardTrackingActive = false;

	bool terriActive = false;

	bbe::TimePoint lastServerReach = bbe::TimePoint::epoch();
	bool serverUnreachableSilenced = false;

	bbe::Monitor monitor;
	float monitorBrightness = 1.0f;
	bool monitorBrightnessOverwrite = false;

	bool highConcentrationMode = false;

	bbe::AdafruitMacroPadRP2040 adafruitMacroPadRP2040;

	bbe::Microphone microphone;
	bbe::Sound microphoneSound;

	bbe::TimePoint nextNewsQuery;
	bool readingNews = false;
	std::future<bbe::Sound> readingNewsFuture;
	bbe::Sound readingNewsSound;
	bbe::SoundInstance readingNewsSoundInstance;
	NewsEntry readingNewsCurrently;
	bool showReadNews = true;

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
				const float white = (cVal > 255 ? (cVal - 255) : 0) / 255.f;

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
					bbe::Crash(bbe::Error::IllegalArgument);
				}

				image.setPixel(x, y, c.asByteColor());
			}
		}

		return image.toIcon();
	}

	void createTrayIcons()
	{
		for (DWORD offset = 0; offset < 512; offset++)
		{
			trayIconsRed.add(createTrayIcon(offset, 0));
			trayIconsGreen.add(createTrayIcon(offset, 1));
			trayIconsBlue.add(createTrayIcon(offset, 2));
		}
	}

	IconCategory getTrayIconCategory() const
	{
		if (isNightTime()) return IconCategory::RED;
		if (tasks.hasCurrentTask()) return IconCategory::BLUE;
		return IconCategory::GREEN;
	}

	bbe::List<HICON>& getTrayIcons()
	{
		switch (getTrayIconCategory())
		{
		case IconCategory::RED: return trayIconsRed;
		case IconCategory::GREEN: return trayIconsGreen;
		case IconCategory::BLUE: return trayIconsBlue;
		}
		bbe::Crash(bbe::Error::IllegalState, "That's not a tray icon category!");
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

		bbe::simpleFile::backup::setBackupPath(generalConfig->backupPath);

		// Initialize ChatGPTComm with the API key if available
		if (!chatGPTConfig->apiKey.isEmpty())
		{
			chatGPTComm.key = chatGPTConfig->apiKey;
		}

		if (generalConfig->windowSet)
		{
			getWindow()->setPos({ generalConfig->windowPosX, generalConfig->windowPosY });
			getWindow()->setSize({ generalConfig->windowSizeX, generalConfig->windowSizeY });

			if (generalConfig->windowMaximized)
			{
				getWindow()->maximize();
			}
		}
	}

	void updateOpenTasksSilenced()
	{
		if (openTasksSilenced)
		{
			openTasksSilencedEnd = bbe::TimePoint().plusHours(1);
		}
		else
		{
			openTasksSilencedIndefinitely = false;
		}
	}

	void adaClipboardKey(int32_t key)
	{
		for (size_t i = 0; i < clipboardContent.getLength(); i++)
		{
			if (clipboardContent[i].adaKey == key)
			{
				setClipboard(clipboardContent[i].content);
				break;
			}
		}
	}

	bbe::TimePoint getNightStart() const
	{
		return bbe::TimePoint::todayAt(22, 00);
	}

	bool isNightTime() const
	{
		bbe::TimePoint now;
		return bbe::TimePoint::todayAt(5, 00) > now || now > getNightStart();
	}

	float getMonitorDim() const
	{
		bbe::TimePoint now;
		if (isNightTime()) return 0.0f;
		const bbe::TimePoint dimStart = getNightStart().plusHours(-2);
		const bbe::TimePoint dimEnd = dimStart.plusHours(1);
		if (now < dimStart) return 1.0f;
		if (now > dimEnd) return 0.0f;

		const auto dimDur = (dimEnd - dimStart).toSeconds();
		const auto dimCur = (now - dimStart).toSeconds();
		return 1.0f - (float)dimCur / dimDur;
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

	auto getServerFuture()
	{
		return bbe::simpleUrlRequest::socketRequestXChaChaAsync(generalConfig->serverAddress, generalConfig->serverPort, generalConfig->serverKeyFilePath, true, true);;
	}

	void minimizeAllWindows()
	{
		EnumWindows(MinimizeWindowCallback, (LPARAM)getNativeWindowHandle());
	}

	virtual void update(float timeSinceLastFrame) override
	{
		beginMeasure("Server Stuff");
		{
			EVERY_SECONDS(5)
			{
				if (!generalConfig->serverAddress.isEmpty()
					&& generalConfig->serverPort != 0
					&& !generalConfig->serverKeyFilePath.isEmpty()
					&& bbe::simpleFile::doesFileExist(generalConfig->serverKeyFilePath))
				{
					static bbe::TimePoint serverDeadline = bbe::TimePoint().plusMinutes(1);
					static auto fut = getServerFuture();
					if (fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
					{
						auto response = fut.get();
						fut = getServerFuture();
						if (response.code == bbe::simpleUrlRequest::SocketRequestXChaChaCode::SUCCESS)
						{
							if (lastServerReach != bbe::TimePoint::epoch())
							{
								auto timeSinceLastReach = bbe::TimePoint() - lastServerReach;
								if (timeSinceLastReach.toMinutes() > 1)
								{
									assetStore::ServerReached()->play();
								}
							}

							lastServerReach = bbe::TimePoint();
							serverDeadline = lastServerReach.plusMinutes(1);

							bbe::String content(response.dataContainer.getRaw());

							auto lines = content.lines(false);

							for (const bbe::String& line : lines)
							{
								int64_t separator = line.search(":");
								if (separator >= 0)
								{
									bbe::String id = line.substring(0, separator);
									if (!seenServerTaskIds.getList().contains({ id }))
									{
										bbe::String task = line.substring(separator + 1, -1);
										BBELOGLN(task);
										tasks.addServerTask(id, task);
										seenServerTaskIds.add({ id });
									}
								}
							}
						}
					}
					if (!serverUnreachableSilenced && serverDeadline.hasPassed())
					{
						assetStore::ServerUnreachable()->play();
						serverDeadline = bbe::TimePoint().plusMinutes(5);
					}
				}
			}
		}

		beginMeasure("AdafruitMacroPadRP2040");
		if (adafruitMacroPadRP2040.isConnected())
		{
			adafruitMacroPadRP2040.update();

			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_10))
			{
				ignoreNight = !ignoreNight;
			}
			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_9))
			{
				openTasksSilenced = !openTasksSilenced;
				updateOpenTasksSilenced();
			}
			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_3))
			{
				adaClipboardKey(1);
			}
			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_4))
			{
				adaClipboardKey(2);
			}
			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_5))
			{
				adaClipboardKey(3);
			}
		}
		else
		{
			adafruitMacroPadRP2040.connect();
		}

		beginMeasure("ChatGPT: Header...");
		if (adafruitMacroPadRP2040.isConnected() && chatGPTComm.isKeySet())
		{
			beginMeasure("ChatGPT: Transcribing...");
			static std::future<bbe::String> transcribeFuture;
			if (microphone.isRecording() && !adafruitMacroPadRP2040.isKeyDown(bbe::RP2040Key::BUTTON_11))
			{
				bbe::Sound sound = microphone.stopRecording();
				transcribeFuture = chatGPTComm.transcribeAsync(sound);
			}
			beginMeasure("ChatGPT: Querying...");
			static std::future<bbe::ChatGPTQueryResponse> queryFuture;
			if (transcribeFuture.valid() && transcribeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				bbe::String transcript = transcribeFuture.get();
				queryFuture = chatGPTComm.queryAsync(transcript);
			}
			beginMeasure("ChatGPT: Synthesizing...");
			static std::future<bbe::Sound> ttsFuture;
			if (queryFuture.valid() && queryFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				bbe::ChatGPTQueryResponse query = queryFuture.get();
				ttsFuture = chatGPTComm.synthesizeSpeechAsync(query.message);
			}
			beginMeasure("ChatGPT: Sound stuff...");
			static bbe::Sound sound;
			if (ttsFuture.valid() && ttsFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				sound = ttsFuture.get();
				sound.play();
			}
			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_11))
			{
				microphone.startRecording();
			}

			if (adafruitMacroPadRP2040.isKeyPressed(bbe::RP2040Key::BUTTON_0))
			{
				chatGPTComm.purgeMemory();
			}
		}

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

		beginMeasure("Reading News");
		{
			if (chatGPTComm.isKeySet() && readingNews)
			{
				if (!readingNewsSoundInstance.isPlaying())
				{
					if (!readingNewsFuture.valid())
					{
						auto news = getUnreadNews();
						if (news.isNull())
						{
							readingNews = false;
							readingNewsCurrently = {};
						}
						else
						{
							bbe::String s = news.title + "... " + news.description;
							readingNewsFuture = chatGPTComm.synthesizeSpeechAsync(s);
							readingNewsCurrently = news;
						}
					}
					else
					{
						if (readingNewsFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
						{
							readingNewsSound = readingNewsFuture.get();
							readingNewsSoundInstance = readingNewsSound.play();
						}
					}
				}
			}
		}

		beginMeasure("Basic Controls");
		static bbe::TimePoint flipToLowEnergyMode = bbe::TimePoint().plusSeconds(2);
		static bbe::TimePoint flipToSuperLowEnergyMode = bbe::TimePoint().plusSeconds(100);
		if (isFocused() || isHovered() || terriActive || adafruitMacroPadRP2040.anyActivity())
		{
			flipToLowEnergyMode      = bbe::TimePoint().plusSeconds(2);
			flipToSuperLowEnergyMode = bbe::TimePoint().plusSeconds(100);
		}
		setTargetFrametime(flipToSuperLowEnergyMode.hasPassed() ? (1.0f / 3.0f) : (flipToLowEnergyMode.hasPassed() ? (1.f / 10.f) : (1.f / 144.f)));
		tabSwitchRequestedLeft = isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::Q);
		tabSwitchRequestedRight = isKeyDown(bbe::Key::LEFT_CONTROL) && isKeyPressed(bbe::Key::E);

		beginMeasure("GlobalKeyboard");
		if (keyboardTrackingActive)
		{
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
		}

		beginMeasure("Play Task Sounds");
		tasks.update();
		if (stopwatches.getList().any([](const Stopwatch& sw) { return sw.shouldPlaySound(); }))
		{
			assetStore::Stopwatch()->play();
		}

		beginMeasure("Tray Icon");
		static IconCategory prevIconCategory = IconCategory::NONE;
		const IconCategory currIconCategory = getTrayIconCategory();
		if (prevIconCategory != currIconCategory || bbe::TrayIcon::isVisible()) bbe::TrayIcon::setIcon(getCurrentTrayIcon());
		prevIconCategory = currIconCategory;

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
					minimizeAllWindows();
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
		processes.update();

		beginMeasure("URL Stuff");
		urls.update();

		beginMeasure("Streak Stuff");
		if ((streakDays.getLength() == 0 || !streakDays.getList().last().day.isToday()) && tasks.isStreakFulfilled())
		{
			streakDays.add(StreakDay{ bbe::TimePoint::todayAt(0, 0, 0) });
		}

		beginMeasure("Beeper");
		if (generalConfig->beepEvery > 0)
		{
			EVERY_MINUTES(generalConfig->beepEvery)
			{
				assetStore::Beep()->play();
			}
		}

		beginMeasure("Monitor Dim");
		if (!monitorBrightnessOverwrite)
		{
			monitorBrightness = getMonitorDim();
		}
		monitor.setBrightness(
			{
			monitorBrightness * generalConfig->baseMonitorBrightness1,
			monitorBrightness * generalConfig->baseMonitorBrightness2,
			monitorBrightness * generalConfig->baseMonitorBrightness3
			}
		);

		beginMeasure("Working Hours");
		if (tasks.hasPotentialTaskComplaint())
		{
			if (openTasksSilencedEnd.hasPassed() && !openTasksSilencedIndefinitely)
			{
				openTasksSilenced = false;
			}
			// Because Process...
			bool shouldPlayOpenTasks = !openTasksSilenced && processes.isGameOn();

			// ... because urls.
			shouldPlayOpenTasks |= !openTasksSilenced && urls.timeWasterFound();
			if (shouldPlayOpenTasks)
			{
				EVERY_MINUTES(15)
				{
					assetStore::OpenTasks()->play();
				}
			}
		}
	}

	bbe::Vector2 drawTabClipboard()
	{
		static ClipboardContent newContent;
		if (ImGui::bbe::InputText("New Content", newContent.content, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
		{
			clipboardContent.add(newContent);
			newContent = ClipboardContent();
		}
		static bool editMode = false;
		ImGui::Checkbox("Edit Mode", &editMode);
		size_t deleteIndex = (size_t)-1;
		bool requiresWrite = false;
		for (size_t i = 0; i < clipboardContent.getLength(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::Button("Delete"))
			{
				deleteIndex = i;
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(100);
			if (ImGui::bbe::combo("##Adakey", { "None", "1", "2", "3" }, clipboardContent[i].adaKey))
			{
				requiresWrite = true;
				if (clipboardContent[i].adaKey != 0)
				{
					for (size_t k = 0; k < clipboardContent.getLength(); k++)
					{
						if (i == k) continue;
						if (clipboardContent[i].adaKey == clipboardContent[k].adaKey)
						{
							clipboardContent[k].adaKey = 0;
						}
					}
				}
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (editMode)
			{
				if (ImGui::bbe::InputText("##Content", clipboardContent[i].content))
				{
					clipboardContent.writeToFile();
				}
			}
			else
			{
				if (ImGui::Button(clipboardContent[i].content))
				{
					setClipboard(clipboardContent[i].content);
				}
			}
			ImGui::bbe::tooltip(bbe::String(clipboardContent[i].content).hardBreakEvery(100));
			ImGui::PopID();
		}
		if (deleteIndex != (size_t)-1)
		{
			clipboardContent.removeIndex(deleteIndex);
		}
		if (requiresWrite)
		{
			clipboardContent.writeToFile();
		}
		return bbe::Vector2(1);
	}

	struct WeatherEntry
	{
		float temperatureC = 0.0f;
		float temperatureCFelt = 0.0f;
		float humidity = 0.0f;
		bbe::TimePoint time;
		float uvIndex = 0.0f;
		float winddir = 0.0f;
		float windspeedKmph = 0.0f;
		float precipMM = 0.0f;
		float chanceOfRain = 0.0f;
	};

	WeatherEntry jsonToWeatherEntry(const nlohmann::json& json, const bbe::TimePoint& day)
	{
		WeatherEntry retVal;

		if      (json.contains("temp_C")) retVal.temperatureC = std::stof(json["temp_C"].get<std::string>());
		else if (json.contains("tempC"))  retVal.temperatureC = std::stof(json["tempC"] .get<std::string>());

		if (json.contains("FeelsLikeC")) retVal.temperatureCFelt = std::stof(json["FeelsLikeC"].get<std::string>());
		if (json.contains("humidity")) retVal.humidity = std::stof(json["humidity"].get<std::string>());

		if (json.contains("localObsDateTime"))
		{
			retVal.time = bbe::TimePoint::fromString(json["localObsDateTime"].get<std::string>().c_str(), "yyyy-MM-dd hh:mm a");
		}
		else if (json.contains("time"))
		{
			retVal.time = bbe::TimePoint::todayAt(std::stoi(json["time"].get<std::string>()) / 100, 0);
			retVal.time = bbe::TimePoint::fromDate(day.getYear(), day.getMonth(), day.getDay(), retVal.time.getHour(), retVal.time.getMinute(), retVal.time.getSecond());
		}

		if (json.contains("uvIndex")) retVal.uvIndex = std::stof(json["uvIndex"].get<std::string>());
		if (json.contains("winddirDegree"))	retVal.winddir = std::stof(json["winddirDegree"].get<std::string>());
		if (json.contains("windspeedKmph"))	retVal.windspeedKmph = std::stof(json["windspeedKmph"].get<std::string>());
		if (json.contains("precipMM")) retVal.precipMM = std::stof(json["precipMM"].get<std::string>());
		if (json.contains("chanceofrain")) retVal.chanceOfRain = std::stof(json["chanceofrain"].get<std::string>());

		return retVal;
	}

	void drawWeatherEntry(bbe::PrimitiveBrush2D& brush, const bbe::Vector2& offset, const WeatherEntry& entry)
	{
		const bbe::List<std::pair<float, bbe::Color>> colorLerps =
		{
			{ -10.0f, bbe::Color(0.8f, 0.8f, 1.0f)},
			{   0.0f, bbe::Color(0.5f, 0.5f, 1.0f)},
			{  22.0f, bbe::Color(0.2f, 1.0f, 0.2f)},
			{  25.0f, bbe::Color(0.8f, 0.8f, 0.2f)},
			{  30.0f, bbe::Color(1.0f, 0.5f, 0.5f)},
			{  40.0f, bbe::Color(0.5f, 0.0f, 0.0f)}
		};

		bbe::String timeString = "";
		timeString += entry.time.getHour();
		timeString += ":00";
		brush.fillText(offset + bbe::Vector2{ 0,  0 }, timeString);
		const bbe::Color tempColor = bbe::Math::multiLerp(colorLerps, entry.temperatureC);
		brush.setColorRGB(tempColor);
		brush.fillText(offset + bbe::Vector2{ 40, 0 }, bbe::String((int)entry.temperatureC) + "C");

		brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
		brush.fillText(offset + bbe::Vector2{ 80,  0 }, "Hum: " + bbe::String((int)entry.humidity));
		brush.fillText(offset + bbe::Vector2{ 150, 0 }, "UV: " + bbe::String((int)entry.uvIndex));
		brush.fillText(offset + bbe::Vector2{ 0, 20 }, "Prcp: " + bbe::String(entry.precipMM).rounded(2));
		brush.fillText(offset + bbe::Vector2{ 79, 20 }, "%Rn: " + bbe::String((int)entry.chanceOfRain));
		brush.fillText(offset + bbe::Vector2{ 138, 20 }, "Wnd: " + bbe::String((int)entry.windspeedKmph));

		brush.sketchRect(offset - bbe::Vector2{ 2, 15 }, { 189, 45 });
	}

	bbe::Vector2 drawWeather(bbe::PrimitiveBrush2D& brush)
	{
		static bbe::TimePoint nextWeatherQuery;
		static std::future<bbe::simpleUrlRequest::UrlRequestResult> future;
		static bbe::List<WeatherEntry> weatherEntries;
		if (ImGui::bbe::InputText("City", weatherConfig->city, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			nextWeatherQuery = bbe::TimePoint();
			weatherConfig.writeToFile();
		}

		if (nextWeatherQuery.hasPassed())
		{
			nextWeatherQuery = bbe::TimePoint().plusHours(1);
			future = bbe::simpleUrlRequest::urlRequestAsync("https://wttr.in/" + weatherConfig->city + "?format=j1");
		}

		if (future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			auto contents = future.get();
			bbe::String s;
			s.append(contents.dataContainer.getRaw(), contents.dataContainer.getLength());

			nlohmann::json j = nlohmann::json::parse(s.getRaw());

			weatherEntries.clear();

			weatherEntries.add(jsonToWeatherEntry(j["current_condition"][0], bbe::TimePoint()));

			nlohmann::json& weather = j["weather"];

			for (size_t i = 0; i < weather.size(); i++)
			{
				bbe::String dayString = weather[i]["date"].get<std::string>().c_str();
				auto dayStringTokens = dayString.split("-");
				if (dayStringTokens.getLength() != 3)
				{
					static bool lengthWarningPrinted = false;
					if (!lengthWarningPrinted)
					{
						lengthWarningPrinted = true;
						BBELOGLN("Warning: Weather reported " << dayStringTokens.getLength() << "tokens!");
					}
					continue;
				}
				bool onlyNumbers = true;
				for (size_t i = 0; i < dayStringTokens.getLength(); i++)
				{
					if (!dayStringTokens[i].isNumber())
					{
						onlyNumbers = false;
						break;;
					}
				}
				if (!onlyNumbers)
				{
					static bool notANumberWarningPrinted = false;
					if (!notANumberWarningPrinted)
					{
						notANumberWarningPrinted = true;
						BBELOGLN("Warning: Weather reported a non number in date!");
					}
					continue;
				}
				bbe::TimePoint day = bbe::TimePoint::fromDate(dayStringTokens[0].toLong(), dayStringTokens[1].toLong(), dayStringTokens[2].toLong());
				nlohmann::json& hourly = weather[i]["hourly"];
				for (size_t k = 0; k < hourly.size(); k++)
				{
					weatherEntries.add(jsonToWeatherEntry(hourly[k], day));
				}
			}
		}

		if (weatherEntries.getLength() > 0)
		{
			int32_t curX = 20;
			int32_t curY = 120;
			bbe::TimePoint previousTime = weatherEntries[0].time;
			for (size_t i = 0; i < weatherEntries.getLength(); i++)
			{
				if (i != 0 && weatherEntries[i].time.hasPassed()) continue;
				if (!previousTime.isSameDay(weatherEntries[i].time))
				{
					curX += 200;
					curY = 120;
				}
				previousTime = weatherEntries[i].time;

				drawWeatherEntry(brush, bbe::Vector2(curX, curY), weatherEntries[i]);

				curY += 50;
			}
		}

		return bbe::Vector2(1, 0.1);
	}

	bbe::Vector2 drawBitcoin()
	{
		static std::future<bbe::simpleUrlRequest::UrlRequestResult> requestFuture;
		static bbe::List<float> times;
		static bbe::List<float> prices;
		EVERY_MINUTES(5)
		{
			requestFuture = bbe::simpleUrlRequest::urlRequestAsync("https://api.coingecko.com/api/v3/coins/bitcoin/market_chart?vs_currency=usd&days=1");
		}
		if (requestFuture.valid() && requestFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			auto val = requestFuture.get();
			if (val.responseCode == 200)
			{
				nlohmann::json json = nlohmann::json::parse(val.dataContainer.getRaw());
				nlohmann::json& pJson = json["prices"];
				times.clear();
				prices.clear();
				for (size_t i = 0; i < pJson.size(); i++)
				{
					times.add(pJson[i][0]);
					prices.add(pJson[i][1]);
				}
			}
		}

		if (prices.getLength() > 0)
		{
			ImGui::Text("Current Price: $%.2f", prices.last());
			if (ImPlot::BeginPlot("Line Plots")) {
				ImPlot::SetupAxes("time", "price");
				ImPlot::PlotLine("Bitcoin", times.getRaw(), prices.getRaw(), times.getLength());
				ImPlot::EndPlot();
			}
			
		}
		return bbe::Vector2(1);
	}

	bool wasNewsRead(NewsEntry& ne)
	{
		if (ne.wasRead) return true;
		for (size_t i = 0; i < readNews.getLength(); i++)
		{
			if (readNews[i] == ne)
			{
				ne.wasRead = true;
				return true;
			}
		}
		return false;
	}

	NewsEntry getUnreadNews()
	{
		for (size_t i = 0; i < newsConfig.getLength(); i++)
		{
			for (size_t k = 0; k < newsConfig[i].newsEntries.getLength(); k++)
			{
				NewsEntry& ne = newsConfig[i].newsEntries[k];
				if (wasNewsRead(ne)) continue;

				ne.wasRead = true;
				readNews.add(ne);
				return ne;
			}
		}
		return NewsEntry();
	}

	bool isValidUrl(const bbe::String& url)
	{
		if (url.containsAny("&|;><"))
		{
			// Naughty characters that could be used to execute other stuff
			// in ShellExecute. Technically & is a legal url char, but none
			// of the RSS feeds I observed contained it. So let's ignore it.
			return false;
		}
		if (url.startsWith("http://") ||
			url.startsWith("https://") ||
			url.startsWith("www.")) {
			return true;
		}
		return false;
	}

	bbe::Vector2 drawNewsConfig()
	{
		bool requiresWrite = false;
		static NewsConfig newNewsConfig;
		ImGui::bbe::InputText("Url", newNewsConfig.url);
		ImGui::bbe::InputText("Iteration Path", newNewsConfig.iterationPath);
		ImGui::bbe::InputText("Title Path", newNewsConfig.titlePath);
		ImGui::bbe::InputText("Description Path", newNewsConfig.descriptionPath);
		ImGui::bbe::InputText("Link Path", newNewsConfig.linkPath);
		if (ImGui::Button("Add Element"))
		{
			newsConfig.add(std::move(newNewsConfig));
			newNewsConfig = {};
			nextNewsQuery = bbe::TimePoint();
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Tagesschau"))
		{
			newNewsConfig.url = "https://www.tagesschau.de/infoservices/alle-meldungen-100~rss2.xml";
			newNewsConfig.iterationPath = "channel/item";
			newNewsConfig.titlePath = "title";
			newNewsConfig.descriptionPath = "description";
			newNewsConfig.linkPath = "link";
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Heise"))
		{
			newNewsConfig.url = "https://www.heise.de/rss/heise-top-atom.xml";
			newNewsConfig.iterationPath = "entry";
			newNewsConfig.titlePath = "title";
			newNewsConfig.descriptionPath = "summary";
			newNewsConfig.linkPath = "link@href";
		}

		ImGui::Separator();
		ImGui::Separator();

		size_t deletionIndex = -1;
		for (size_t i = 0; i < newsConfig.getLength(); i++)
		{
			ImGui::Separator();
			ImGui::PushID(i);
			if (ImGui::Button("Delete")) deletionIndex = i;
			requiresWrite |= ImGui::bbe::InputText("Url", newsConfig[i].url);
			requiresWrite |= ImGui::bbe::InputText("Iteration Path", newsConfig[i].iterationPath);
			requiresWrite |= ImGui::bbe::InputText("Title Path", newsConfig[i].titlePath);
			requiresWrite |= ImGui::bbe::InputText("Description Path", newsConfig[i].descriptionPath);
			requiresWrite |= ImGui::bbe::InputText("Link Path", newsConfig[i].linkPath);
			ImGui::PopID();
		}

		if (deletionIndex != -1)
		{
			newsConfig.removeIndex(deletionIndex);
			nextNewsQuery = bbe::TimePoint();
		}

		if (requiresWrite)
		{
			newsConfig.writeToFile();
			nextNewsQuery = bbe::TimePoint();
		}

		return bbe::Vector2(1);
	}

	bbe::String extractInfoFromXmlElement(const tinyxml2::XMLElement* element, const bbe::String& path)
	{
		const size_t ats = path.count("@");
		if (ats > 1) return ""; // Only one at allowed.

		auto atSplit = path.split("@");
		auto slashSplit = atSplit[0].split("/");

		for (size_t i = 0; i < slashSplit.getLength(); i++)
		{
			element = element->FirstChildElement(slashSplit[i].getRaw());
			if (!element) return "";
		}

		const char* val = nullptr;
		if (atSplit.getLength() == 1)
		{
			val = element->GetText();
		}
		else
		{
			val = element->Attribute(atSplit[1].getRaw());
		}
		if (!val) return "";
		return val;
	}

	bbe::Vector2 drawNews()
	{
		ImGui::Checkbox("Reading news", &readingNews);
		ImGui::Checkbox("Show read news", &showReadNews);

		if (nextNewsQuery.hasPassed())
		{
			nextNewsQuery = bbe::TimePoint().plusMinutes(5);
			for (size_t i = 0; i < newsConfig.getLength(); i++)
			{
				newsConfig[i].queryFuture = bbe::simpleUrlRequest::urlRequestAsync(newsConfig[i].url).share();
			}
		}

		bool didFail = false;
		for (size_t i = 0; i < newsConfig.getLength(); i++)
		{
			if (newsConfig[i].queryFuture.valid() && newsConfig[i].queryFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				auto response = newsConfig[i].queryFuture.get();
				newsConfig[i].queryFuture = std::shared_future<bbe::simpleUrlRequest::UrlRequestResult>();
				newsConfig[i].queryData = response.dataContainer.getRaw();

				tinyxml2::XMLDocument doc;
				if (doc.Parse(newsConfig[i].queryData.getRaw()) != tinyxml2::XML_SUCCESS)
				{
					didFail = true;
					goto fail;
				}

				tinyxml2::XMLElement* iterationElement = doc.RootElement();

				auto path = newsConfig[i].iterationPath.split("/");
				if (path.getLength() == 0)
				{
					didFail = true;
					goto fail;
				}
				for (size_t i = 0; i < path.getLength(); i++)
				{
					iterationElement = iterationElement->FirstChildElement(path[i].getRaw());
					if (!iterationElement)
					{
						didFail = true;
						goto fail;
					}
				}

				newsConfig[i].newsEntries.clear();
				while (iterationElement)
				{
					NewsEntry entry;
					entry.title       = extractInfoFromXmlElement(iterationElement, newsConfig[i].titlePath);
					entry.description = extractInfoFromXmlElement(iterationElement, newsConfig[i].descriptionPath);
					entry.link        = extractInfoFromXmlElement(iterationElement, newsConfig[i].linkPath);
					if (!isValidUrl(entry.link)) entry.link = "";
					newsConfig[i].newsEntries.add(entry);

					iterationElement = iterationElement->NextSiblingElement(path.last().getRaw());
				}
			}
		}
	fail:
		if (didFail)
		{
			static bool reportedFailure = false;
			if (!reportedFailure)
			{
				reportedFailure = true;
				BBELOGLN("Failed to parse news!");
			}
		}

		for (size_t i = 0; i < newsConfig.getLength(); i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 1.0f, 1.0f));
			ImGui::Text("%s", newsConfig[i].url.getRaw());
			ImGui::PopStyleColor();
			for (size_t k = 0; k < newsConfig[i].newsEntries.getLength(); k++)
			{
				float colorMult = 1.0f;
				bool newsWasRead = false;
				if (wasNewsRead(newsConfig[i].newsEntries[k]))
				{
					colorMult = 0.3f;
					newsWasRead = true;
				}
				if (readingNewsCurrently == newsConfig[i].newsEntries[k] && (readingNews || readingNewsSoundInstance.isPlaying()))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 1.0f, 0.8f, 1.0f));
					colorMult = 1.0f;
					newsWasRead = false;
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f * colorMult, 1.0f * colorMult, 1.0f * colorMult, 1.0f));
				}
				if (!showReadNews && newsWasRead)
				{
					ImGui::PopStyleColor();
					continue;
				}
				ImGui::TextWrapped("%s", newsConfig[i].newsEntries[k].title.getRaw());
				ImGui::Indent(10.0f);
				ImGui::TextWrapped("%s", newsConfig[i].newsEntries[k].description.getRaw());
				ImGui::PopStyleColor();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f * colorMult, 0.5f * colorMult, 1.0f * colorMult, 1.0f));
				if (newsConfig[i].newsEntries[k].link.getLength() > 0)
				{
					if (ImGui::Selectable(newsConfig[i].newsEntries[k].link.getRaw()))
					{
						ShellExecuteA(nullptr, "open", newsConfig[i].newsEntries[k].link.getRaw(), nullptr, nullptr, SW_SHOWNORMAL);
					}
				}
				ImGui::PopStyleColor();
				ImGui::Indent(-10.0f);
			}
		}

		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabConsole()
	{
		const auto& log = bbe::logging::getLog();
		static int64_t sliderVal = 0;
		const int64_t min = 0;
		const int64_t max = log.getLength() - 2;
		ImGui::VSliderScalar("##Scrollbar", { 25, ImGui::GetWindowHeight() - 50 }, ImGuiDataType_S64, &sliderVal, &max, &min);
		constexpr int64_t wheelSpeed = 5;
		if (getMouseScrollY() > 0)
		{
			sliderVal -= wheelSpeed;
		}
		else if (getMouseScrollY() < 0)
		{
			sliderVal += wheelSpeed;
		}
		sliderVal = bbe::Math::clamp(sliderVal, min, max);
		bbe::String txt = "";
		for (size_t i = 0; i < 1024; i++)
		{
			const size_t index = i + sliderVal;
			if (index >= log.getLength()) break;
			std::lock_guard _(log);
			txt += log[index];
			txt += "\n";
		}
		ImGui::SameLine();
		ImGui::Text(txt.getRaw());


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
			bbe::simpleFile::backup::setBackupPath(generalConfig->backupPath);
		}
		generalConfigChanged |= ImGui::bbe::InputText("Server Address", generalConfig->serverAddress, ImGuiInputTextFlags_EnterReturnsTrue);
		generalConfigChanged |= ImGui::InputInt("Server Port", &generalConfig->serverPort);
		generalConfigChanged |= ImGui::bbe::InputText("Server Key Path", generalConfig->serverKeyFilePath, ImGuiInputTextFlags_EnterReturnsTrue);

		generalConfigChanged |= ImGui::SliderFloat("Base Monitor Brightness 1", &generalConfig->baseMonitorBrightness1, 0.0f, 1.0f);
		generalConfigChanged |= ImGui::SliderFloat("Base Monitor Brightness 2", &generalConfig->baseMonitorBrightness2, 0.0f, 1.0f);
		generalConfigChanged |= ImGui::SliderFloat("Base Monitor Brightness 3", &generalConfig->baseMonitorBrightness3, 0.0f, 1.0f);
		
		if (ImGui::Button("Remember Window Position"))
		{
			generalConfig->windowSet = true;
			generalConfig->windowPosX = getWindow()->getPos().x;
			generalConfig->windowPosY = getWindow()->getPos().y;
			generalConfig->windowSizeX = getWindow()->getSize().x;
			generalConfig->windowSizeY = getWindow()->getSize().y;
			generalConfig->windowMaximized = getWindow()->isMaximized();
			generalConfigChanged = true;
		}

		if (generalConfigChanged)
		{
			generalConfig.writeToFile();
		}
		return bbe::Vector2(1);
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
		static std::atomic<bool> loaded(false);
		static float multiplier = 1.0f;
		static std::atomic<float> progress(0.0f);
		static std::future<void> computationFuture;

		ImGui::SliderFloat("Multiplier", &multiplier, 0.0f, 1.0f);

		bool computationInProgress = computationFuture.valid() && computationFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

		ImGui::BeginDisabled(computationInProgress);
		if (ImGui::Button("Do it!"))
		{
			if (!computationInProgress)
			{
				// Start the computation
				progress = 0.0f;
				loaded = false;

				computationFuture = std::async(std::launch::async, [&]() {

					bbe::List<bbe::Vector2> positions;
					float maxX = 0;
					float maxY = 0;
					float minX = 0;
					float minY = 0;

					bbe::List<bbe::String> files;
					bbe::simpleFile::forEachFile("mousePositions", [&](const bbe::String& path) {
						files.add(path);
						});

					size_t totalFiles = files.getLength();
					for (size_t idx = 0; idx < totalFiles; ++idx)
					{
						const bbe::String& path = files[idx];

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

						progress = 10.0f * (float)(idx + 1) / (float)totalFiles; // Reading files progress
					}

					bbe::Grid<float> grid(maxX - minX + 1, maxY - minY + 1);
					float maxValue = 0;

					size_t totalPositions = positions.getLength();
					for (size_t i = 0; i < totalPositions; i++)
					{
						bbe::Vector2& p = positions[i];
						grid[p.x - minX][p.y - minY]++;
						maxValue = bbe::Math::max(maxValue, grid[p.x - minX][p.y - minY]);

						if (i % 1000 == 0)
						{
							progress = 10.0f + 40.0f * (float)(i + 1) / (float)totalPositions; // Processing positions progress
						}
					}

					image = bbe::Image(maxX - minX + 1, maxY - minY + 1);

					size_t gridWidth = grid.getWidth();
					size_t gridHeight = grid.getHeight();

					for (size_t x = 0; x < gridWidth; x++)
					{
						for (size_t y = 0; y < gridHeight; y++)
						{
							grid[x][y] *= multiplier;
							image.setPixel(x, y, bbe::Color(grid[x][y] > 1.f ? 1.f : grid[x][y]).asByteColor());
						}

						if (x % 10 == 0)
						{
							progress = 50.0f + 50.0f * (float)(x + 1) / (float)gridWidth; // Image creation progress
						}
					}

					loaded = true;
					progress = 100.0f;
					});
			}
		}
		ImGui::EndDisabled();

		if (computationInProgress)
		{
			ImGui::Text("Progress: %.2f%%", progress.load());
		}
		else
		{
			ImGui::Text("");
		}
		if (!computationInProgress && loaded)
		{
			brush.drawImage(0, 200, 800, 400, image);
		}

		bbe::Vector2 globalMouse = getMouseGlobal();
		ImGui::Text("Global Mouse: %f/%f", globalMouse.x, globalMouse.y);

		return bbe::Vector2(1.0f, 0.14f);
	}

	bbe::Vector2 drawTabKeyboardTracking(bbe::PrimitiveBrush2D& brush)
	{
		static bool normalize = true;
		ImGui::Checkbox("Normalize", &normalize);
		if (ImGui::Checkbox("Active", &keyboardTrackingActive))
		{
			if (keyboardTrackingActive) globalKeyboard.init();
			else globalKeyboard.uninit();
		}

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
		bool usedKeys[(size_t)bbe::Key::LAST] = {};
		for (size_t i = 0; i < keys.getLength(); i++)
		{
			DrawnKey& k = keys[i];
			k.value = keyboardTracker->keyPressed[(size_t)k.key];
			min = bbe::Math::min(min, k.value);
			max = bbe::Math::max(max, k.value);
			usedKeys[(size_t)k.key] = true;
		}

		if (!normalize) min = 0.0f;

		for (size_t i = 0; i < keys.getLength(); i++)
		{
			DrawnKey& k = keys[i];
			k.value = (k.value - min) / (max - min);
			brush.setColorRGB(bbe::Color(k.value, k.value, k.value));
			brush.fillText(30 + k.pos.x * 60, 400 + k.pos.y * 60, bbe::keyCodeToString(k.key), 40);
		}

		for (size_t i = 0; i < (size_t)bbe::Key::LAST; i++)
		{
			if (!usedKeys[i] && keyboardTracker->keyPressed[i] > 0)
			{
				ImGui::Text("%s: %d", bbe::keyCodeToString((bbe::Key)i).getRaw(), keyboardTracker->keyPressed[i]);
			}
		}

		return bbe::Vector2(1.0f, 0.2f);
	}

	bbe::Vector2 drawTabStreaks(bbe::PrimitiveBrush2D& brush)
	{
		const int32_t year = bbe::TimePoint().getYear();
		for (int32_t i = 1; i <= 12; i++)
		{
			const int32_t days = bbe::TimePoint::getDaysInMonth(year, i);
			for (int32_t k = 1; k <= days; k++)
			{
				bbe::TimePoint tp = bbe::TimePoint::fromDate(year, i, k);
				bool isStreakDay = false;
				for (size_t m = 0; m < streakDays.getLength(); m++)
				{
					const bbe::TimePoint& cDay = streakDays[m].day;
					if (cDay.getYear() == tp.getYear() && cDay.getMonth() == tp.getMonth() && cDay.getDay() == tp.getDay())
					{
						isStreakDay = true;
						break;
					}
				}

				if (isStreakDay)
				{
					brush.setColorRGB(1.0f, 1.0f, 0.5f, 1.0f);
				}
				else if (tp.isToday())
				{
					brush.setColorRGB(0.5f, 0.5f, 1, 1);
				}
				else if (tp.hasPassed())
				{
					brush.setColorRGB(1, 1, 1, 1);
				}
				else
				{
					brush.setColorRGB(0.3f, 0.3f, 0.3f, 1);
				}
				brush.sketchRect(5 - 2 + k * 23, 5 + i * 30, 20, 20);
				brush.fillText(15 - 2 + k * 23, 20 + i * 30, bbe::String(k), 20, bbe::Anchor::BOTTOM_CENTER);
			}
		}

		return bbe::Vector2(1.0f, 0.0f);
	}

#if 0
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
				bbe::Colori c = screenshot.getPixel(0, 70 - 10 * i);
				float val = (c.r + c.g + c.b) / 3.0f;
				if (val < 128) break;
			}
			static int previousI = 0;
			if (previousI != i)
			{
				if (i == 0) assetStore::One()  ->play();
				if (i == 1) assetStore::Two()  ->play();
				if (i == 2) assetStore::Three()->play();
				if (i == 3) assetStore::Four() ->play();
				if (i == 4) assetStore::Five() ->play();
				if (i == 5) assetStore::Six()  ->play();
				if (i == 6) assetStore::Seven()->play();
				if (i == 7) assetStore::Eight()->play();
				previousI = i;
			}
		}
		return bbe::Vector2(1.0f);
	}
#endif

	bbe::Vector2 drawTabRememberLists()
	{
		ImGui::Text("Hold SHIFT to delete stuff.");
		bool requiresWrite = false;
		size_t listDeleteIndex = (size_t)-1;
		for (size_t i = 0; i < rememberLists.getLength(); i++)
		{
			ImGui::PushID(i);
			if (rememberLists[i].entries.getLength() == 0)
			{
				if (ImGui::bbe::securityButton("Delete", ImGui::bbe::SecurityButtonFlags_DontShowWOSecurityButton))
				{
					listDeleteIndex = i;
				}
				ImGui::SameLine();
			}
			if (ImGui::TreeNode((void*)(intptr_t)i, "%s", rememberLists[i].title.getRaw()))
			{
				size_t subDeleteIndex = (size_t)-1;
				for (size_t k = 0; k < rememberLists[i].entries.getLength(); k++)
				{
					ImGui::PushID(k);
					if (ImGui::bbe::securityButton("Delete", ImGui::bbe::SecurityButtonFlags_DontShowWOSecurityButton))
					{
						subDeleteIndex = k;
					}
					ImGui::SameLine();
					if (ImGui::bbe::clickableText("%s", rememberLists[i].entries[k].getRaw()))
					{
						ImGui::SetClipboardText(rememberLists[i].entries[k].getRaw());
					}
					ImGui::PopID();
				}
				if (ImGui::bbe::InputText("New Entry", rememberLists[i].newEntryBuffer, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					rememberLists[i].entries.add(rememberLists[i].newEntryBuffer);
					rememberLists[i].newEntryBuffer = {};
					requiresWrite = true;
				}
				if (subDeleteIndex != (size_t)-1)
				{
					rememberLists[i].entries.removeIndex(subDeleteIndex);
					requiresWrite = true;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		static bbe::String newListBuffer;
		if (ImGui::bbe::InputText("New List", newListBuffer, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			RememberList newList;
			newList.title = newListBuffer;
			rememberLists.add(newList);
			newListBuffer = {};
		}
		if (listDeleteIndex != (size_t)-1)
		{
			rememberLists.removeIndex(listDeleteIndex);
		}
		if (requiresWrite)
		{
			rememberLists.writeToFile();
		}

		return bbe::Vector2(1);
	}

	bbe::Vector2 drawMicrophoneTest()
	{
		if (microphone.isRecording())
		{
			if (ImGui::Button("Stop"))
			{
				microphoneSound = microphone.stopRecording();
			}
		}
		else
		{
			if (ImGui::Button("Start"))
			{
				microphone.startRecording();
			}
		}

		if (ImGui::Button("Play!"))
		{
			microphoneSound.play();
		}

		if (ImGui::Button("Save!"))
		{
			auto wav = microphoneSound.toWav();
			bbe::simpleFile::writeBinaryToFile("Debug.wav", wav);
		}

		static std::future<bbe::String> transcribeFuture;
		if (ImGui::Button("Transcribe"))
		{
			transcribeFuture = chatGPTComm.transcribeAsync(microphoneSound);
		}

		static bbe::String transcription;
		if (transcribeFuture.valid() && transcribeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			transcription = transcribeFuture.get();
		}

		ImGui::Text(transcription);

		return bbe::Vector2(1);
	}

	bbe::Vector2 drawTabChatGPT()
	{
		if (ImGui::bbe::InputText("API Key", chatGPTConfig->apiKey, ImGuiInputTextFlags_Password))
		{
			chatGPTConfig.writeToFile();
			// Set the API key in chatGPTComm
			chatGPTComm.key = chatGPTConfig->apiKey;
		}
		
		static bbe::String ttsInput;
		if (ImGui::bbe::InputText("TTS Test", ttsInput, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (chatGPTComm.isKeySet())
			{
				ttsSoundSet = false;
				chatGPTTTSFuture = chatGPTComm.synthesizeSpeechAsync(ttsInput);
			}
		}

		if (chatGPTTTSFuture.valid() && chatGPTTTSFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			currentTTSSound = chatGPTTTSFuture.get();
			ttsSoundSet = true;
		}

		ImGui::BeginDisabled(!ttsSoundSet);
		ImGui::SameLine();
		if (ImGui::Button("Play it!"))
		{
			currentTTSSound.play();
		}
		ImGui::EndDisabled();

		// Input field for user's message
		static bbe::String userInput;
		static bbe::String errorString;
		if (ImGui::bbe::InputText("Your Message", userInput, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// When the user presses Enter, send the message
			if (chatGPTComm.isKeySet())
			{
				// Call queryAsync to avoid blocking UI
				try
				{
					chatGPTFuture = chatGPTComm.queryAsync(userInput);
					userInput = "";
					errorString = "";
				}
				catch (const std::exception& e)
				{
					errorString = "Error initiating query";
					errorString += e.what();
				}
			}
			else
			{
				errorString = "Please set the API key.";
			}
		}
		ImGui::Text(errorString);

		// Check if the future is ready
		bool waitingPrinted = false;
		if (chatGPTFuture.valid())
		{
			std::future_status status = chatGPTFuture.wait_for(std::chrono::milliseconds(0));
			if (status == std::future_status::ready)
			{
				try
				{
					// Get the response
					bbe::ChatGPTQueryResponse response = chatGPTFuture.get();
					// The response is already added to chatGPTComm.history
				}
				catch (const std::exception& e)
				{
					ImGui::Text("Error: %s", e.what());
				}
			}
			else
			{
				// Optionally, display a "Loading..." indicator
				ImGui::Text("Waiting for response...");
				waitingPrinted = true;
			}
		}
		if(!waitingPrinted) ImGui::Text(" "); // So that the "Waiting for response" doesn't move part of the GUI.

		// Provide a button to purge the conversation
		if (ImGui::Button("Clear Conversation"))
		{
			chatGPTComm.purgeMemory();
		}

		// Display the conversation history
		ImGui::BeginChild("Conversation", ImVec2(0, 0), true);

		// Skip the system message (first message)
		for (size_t i = 0; i < chatGPTComm.history.size(); ++i)
		{
			const auto& message = chatGPTComm.history[i];
			std::string role = message["role"];
			std::string content = message["content"];
			if (role == "system")
			{
				// Skip system message
				continue;
			}
			else if (role == "user")
			{
				// Display user message
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8, 0.8, 1, 1)); // Blue color
				ImGui::TextWrapped("You: %s", content.c_str());
				ImGui::PopStyleColor();
			}
			else if (role == "assistant")
			{
				// Display assistant message
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 1, 0.8f, 1)); // Green color
				ImGui::TextWrapped("Assistant: %s", content.c_str());
				ImGui::PopStyleColor();
			}
		}

		ImGui::EndChild();

		return bbe::Vector2(1);
	}

	bbe::Vector2 drawAdafruitMacroPadRP2040(bbe::PrimitiveBrush2D& brush)
	{
		if (adafruitMacroPadRP2040.isKeyDown(bbe::RP2040Key::BUTTON_AUDIO))
		{
			brush.setColorRGB(0.5f, 0.5f, 1.0f);
		}
		else
		{
			brush.setColorRGB(1, 1, 1);
		}

		brush.fillText(350, 100, bbe::String(adafruitMacroPadRP2040.getRotationValue()));
		for (int i = 0; i < 12; i++)
		{
			brush.setColorRGB(1, 1, 1);
			int x = i % 3;
			int y = i / 3;
			constexpr int rectSize = 100;
			bbe::Rectangle r(100 + x * (rectSize + 4), 140 + y * (rectSize + 4), rectSize, rectSize);
			brush.sketchRect(r);
			if (adafruitMacroPadRP2040.isKeyDown((bbe::RP2040Key)i))
			{
				r.shrinkInPlace(5);
				brush.setColorRGB(0.5f, 0.5f, 1.0f);
				brush.fillRect(r);
			}
		}
		return bbe::Vector2(0);
	}

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		bool shouldMinimize = false;
		beginMeasure("Draw main window");
		static bbe::Vector2 sizeMult(1.0f, 1.0f);
		ImGuiViewport viewport = *ImGui::GetMainViewport();
		viewport.WorkSize.x *= 0.6f * sizeMult.x;
		viewport.WorkSize.y *= sizeMult.y;
		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(viewport.WorkSize);
		static Tab previousTab;
		ImGui::Begin("MainWindow", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | (previousTab.title == bbe::String("Cnsl") ? ImGuiWindowFlags_NoScrollWithMouse : 0));
		{
			static bbe::List<Tab> tabs =
			{
				Tab{"VTasks",    "View Tasks",     [&]() { return tasks.drawTabViewTasks(); }},
				Tab{"ETasks",    "Edit Tasks",     [&]() { return tasks.drawTabEditTasks(); }},
				Tab{"Clpbrd",    "Clipboard",      [&]() { return drawTabClipboard(); }},
				//Tab{"Brn-T",     "Brain-Teaser",   [&]() { return brainTeasers.drawTabBrainTeasers(brush); }},
				Tab{"Stpwtch",   "Stopwatch",      [&]() { return drawTabStopwatch(); }},
				Tab{"MsTrck",    "Mouse Track",    [&]() { return drawTabMouseTracking(brush); }},
				Tab{"KybrdTrck", "Keyboard Track", [&]() { return drawTabKeyboardTracking(brush); }},
#if 0
				Tab{"Terri",     "Territorial",    [&]() { return drawTabTerri(brush); }},
#endif
				Tab{"Strks",     "Streaks",        [&]() { return drawTabStreaks(brush); }},
				Tab{"Lsts",      "Lists",          [&]() { return drawTabRememberLists(); }},
				Tab{"GPT",       "ChatGPT",        [&]() { return drawTabChatGPT(); }},
				//Tab{"Mic",       "Microphone Test",[&]() { return drawMicrophoneTest(); }},
				//Tab{"Ada",       "AdafruitMacroPadRP2040", [&]() { return drawAdafruitMacroPadRP2040(brush); }},
				Tab{"Wthr",      "Weather",        [&]() { return drawWeather(brush); }},
				Tab{"BTC",       "Bitcoin",        [&]() { return drawBitcoin(); }},
				Tab{"VNews",     "View News",      [&]() { return drawNews(); }},
				Tab{"ENews",     "Edit News",      [&]() { return drawNewsConfig(); }},
				Tab{"Cnsl",      "Console",        [&]() { return drawTabConsole(); }},
				Tab{"Cnfg",      "Config",         [&]() { return drawTabConfig(); }},
			};
			static size_t previousShownTab = 0;
			DrawTabResult dtr = drawTabs(tabs, &previousShownTab, tabSwitchRequestedLeft, tabSwitchRequestedRight);
			sizeMult = dtr.sizeMult;
			previousTab = dtr.tab;
		}
		ImGui::End();

		beginMeasure("Draw info window");
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
			ImGui::Text(bbe::simpleFile::backup::async::hasOpenIO() ? "Saving" : "Done");
			bbe::String s = "Night Start in: " + (getNightStart() - bbe::TimePoint()).toString();
			ImGui::Text(s.getRaw());
			ImGui::bbe::tooltip(getNightStart().toString().getRaw());

			tasks.drawUndoRedoButtons();

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
			static bool updatePathNewer = false;
			// Avoiding multiple IO calls.
			EVERY_SECONDS(10)
			{
				if (!updatePathExists)
				{
					updatePathExists = bbe::simpleFile::doesFileExist(generalConfig->updatePath);
				}
				else
				{
					const auto updateModTime = bbe::simpleFile::getLastModifyTime(generalConfig->updatePath);
					const auto thisModTime = bbe::simpleFile::getLastModifyTime(bbe::simpleFile::getExecutablePath());
					if (updateModTime && thisModTime)
					{
						updatePathNewer = updateModTime > thisModTime;
					}
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
				if (updatePathNewer)
				{
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "(!)");
					ImGui::bbe::tooltip("The update path is newer than this version!");
				}
			}

			bbe::simpleProcess::drawElevationButton(this);

			if (ImGui::Button("Play Sound"))
			{
				assetStore::NewTask()->play();
			}
			ImGui::SameLine();
			if (ImGui::Button("Restart Sound System"))
			{
				restartSoundSystem();
			}
			if (ImGui::Button("Print something to log"))
			{
				static int64_t counter = 0;
				for (int i = 0; i < 4000000; i++)
				{
					bbe::String s = "Something ";
					s += counter;
					counter++;
					BBELOGLN(s);
				}
			}
			if (ImGui::Button("Minimize all Windows"))
			{
				shouldMinimize = true;
			}

			static bool unlockCrashButton = false;
			ImGui::Checkbox("Unlock Crash Buttons", &unlockCrashButton);
			ImGui::SameLine();
			ImGui::BeginDisabled(!unlockCrashButton);
			if (ImGui::Button("Segv!"))
			{
				*((volatile int*)0);
			}
			ImGui::SameLine();
			if (ImGui::Button("Crash!"))
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
			ImGui::SameLine();
			if (ImGui::Button("Exception!"))
			{
				throw 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Free Illegal!"))
			{
				free((void*)0x12345678);
			}
			ImGui::EndDisabled();

			if (ImGui::Checkbox("Silence Open Task (1 Hour)", &openTasksSilenced))
			{
				updateOpenTasksSilenced();
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(!openTasksSilenced);
			ImGui::Checkbox("Indefinitely ", &openTasksSilencedIndefinitely);
			ImGui::EndDisabled();

			if (openTasksSilenced && !openTasksSilencedIndefinitely && !openTasksSilencedEnd.hasPassed())
			{
				const bbe::TimePoint now;
				const bbe::Duration duration = openTasksSilencedEnd - now;
				ImGui::SameLine();
				ImGui::Text("Time left: %s", duration.toString().getRaw());
			}

			ImGui::Checkbox("Ignore Night", &ignoreNight);

			ImGui::BeginDisabled(!bbe::simpleProcess::isRunAsAdmin());
			if (ImGui::Checkbox("Enable High Concentration Mode", &highConcentrationMode))
			{
				if (highConcentrationMode)
				{
					urls.enableHighConcentrationMode();
				}
				else
				{
					urls.disableHighConcentrationMode();
				}
			}
			ImGui::EndDisabled();

			ImGui::Checkbox("Let me prepare", &tasks.forcePrepare); ImGui::bbe::tooltip("Make tasks advancable, even before late time happens.");
			bbe::String serverUnreachableString = "Silence Server Unreachable. Last reach: ";
			if (lastServerReach == bbe::TimePoint::epoch())
			{
				serverUnreachableString += "Never";
			}
			else
			{
				serverUnreachableString += (bbe::TimePoint() - lastServerReach).toString();
			}
			ImGui::Checkbox(serverUnreachableString.getRaw(), &serverUnreachableSilenced);
			ImGui::Checkbox("Show Debug Stuff", &showDebugStuff);

			ImGui::Checkbox("Overwrite Monitor Brightness", &monitorBrightnessOverwrite);
			ImGui::BeginDisabled(!monitorBrightnessOverwrite);
			ImGui::SameLine();
			ImGui::PushItemWidth(100);
			ImGui::SliderFloat("##Monitor Brightness", &monitorBrightness, 0.0, 1.0);
			ImGui::PopItemWidth();
			ImGui::EndDisabled();

			ImGui::NewLine();
			ImGui::Text("Playing sounds: %d", (int)getAmountOfPlayingSounds());
			drawMeasurement();
		}
		ImGui::End();

		beginMeasure("Draw process window");
		viewport.WorkPos.y = viewport.WorkSize.y;
		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(viewport.WorkSize);
		ImGui::Begin("Processes", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			processes.drawGui();
		}
		ImGui::End();


		beginMeasure("Draw url window");
		viewport.WorkPos.y = viewport.WorkSize.y * 2;
		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(viewport.WorkSize);
		ImGui::Begin("URLs", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			urls.drawGui();
		}
		ImGui::End();

		beginMeasure("Draw debug windows");
		if (showDebugStuff)
		{
			ImGui::ShowDemoWindow();
			ImPlot::ShowDemoWindow();
		}

		if (shouldMinimize)
		{
			// We need to delay this to the end, or else dear ImGui gets confused.
			minimizeAllWindows();
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
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF); // See: https://stackoverflow.com/questions/30413066/how-do-i-diagnose-heap-corruption-errors-on-windows
	HWND hWnd = GetConsoleWindow();
	FreeConsole();
	MyGame* mg = new MyGame();
	mg->start(1280, 720, "M.O.THE.R - Memory of the repetitive");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
