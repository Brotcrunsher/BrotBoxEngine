#include "BBE/BrotBoxEngine.h"
#include <iostream>
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

//TODO: If openal is multithreaded, then why don't we launch static sounds on the main thread and push the info over to the audio thread for later processing?
//      Careful when doing this ^^^^^^ - Audio Restart on device change?
//TODO: Time selector (next to date picker)
//TODO: Non eearly tasks should be greyed out during early hours
//TODO: Serializable List/Object should somehow handle versions... it's really complicated to do that within the nice BBE_SERIALIZABLE_DATA macro though.
//TODO: Nighttime configurable
//TODO: Latetime configurable
//TODO: Time left was negative. Why? It was evening and no tasks were remaining.
//TODO: Add a new checkbox to tasks with contingent. When that checkbox is ticked, then adding to the contingent should be paused if the windows screen is locked.

struct ClipboardContent
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), content)
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
		((int32_t), serverPort, 3490)
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

enum class IconCategory
{
	NONE,
	RED,
	GREEN,
	BLUE,
};

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

	bbe::ChatGPTComm chatGPTComm; // ChatGPT communication object
	std::future<bbe::ChatGPTQueryResponse> chatGPTFuture; // Future for async ChatGPT queries

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
		monitor.setBrightness(monitorBrightness);

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
		for (size_t i = 0; i < clipboardContent.getLength(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::Button("Delete"))
			{
				deleteIndex = i;
			}
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

	bbe::Vector2 drawTabChatGPT()
	{
		if (ImGui::bbe::InputText("API Key", chatGPTConfig->apiKey, ImGuiInputTextFlags_Password))
		{
			chatGPTConfig.writeToFile();
			// Set the API key in chatGPTComm
			chatGPTComm.key = chatGPTConfig->apiKey;
		}

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

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
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
				Tab{"Brn-T",     "Brain-Teaser",   [&]() { return brainTeasers.drawTabBrainTeasers(brush); }},
				Tab{"Stpwtch",   "Stopwatch",      [&]() { return drawTabStopwatch(); }},
				Tab{"MsTrck",    "Mouse Track",    [&]() { return drawTabMouseTracking(brush); }},
				Tab{"KybrdTrck", "Keyboard Track", [&]() { return drawTabKeyboardTracking(brush); }},
#if 0
				Tab{"Terri",     "Territorial",    [&]() { return drawTabTerri(brush); }},
#endif
				Tab{"Strks",     "Streaks",        [&]() { return drawTabStreaks(brush); }},
				Tab{"Lsts",      "Lists",          [&]() { return drawTabRememberLists(); }},
				Tab{"GPT",       "ChatGPT",        [&]() { return drawTabChatGPT(); }},
				Tab{"Cnsl",      "Console",        [&]() { return drawTabConsole(); }},
				Tab{"Cnfg",      "Config",         [&]() { return drawTabConfig(); }},
			};
			static size_t previousShownTab = 0;
			DrawTabResult dtr = drawTabs(tabs, &previousShownTab, tabSwitchRequestedLeft, tabSwitchRequestedRight);
			sizeMult = dtr.sizeMult;
			previousTab = dtr.tab;
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
				if (openTasksSilenced)
				{
					openTasksSilencedEnd = bbe::TimePoint().plusHours(1);
				}
				else
				{
					openTasksSilencedIndefinitely = false;
				}
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(!openTasksSilenced);
			ImGui::Checkbox("Indefinitely ", &openTasksSilencedIndefinitely);
			ImGui::EndDisabled();

			if (openTasksSilenced && !openTasksSilencedIndefinitely)
			{
				const bbe::TimePoint now;
				const bbe::Duration duration = openTasksSilencedEnd - now;
				ImGui::SameLine();
				ImGui::Text("Time left: %s", duration.toString().getRaw());
			}

			ImGui::Checkbox("Ignore Night", &ignoreNight);
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
			if (ImGui::SliderFloat("##Monitor Brightness", &monitorBrightness, 0.0, 1.0))
			{
				monitor.setBrightness(monitorBrightness);
			}
			ImGui::PopItemWidth();
			ImGui::EndDisabled();

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
			processes.drawGui();
		}
		ImGui::End();


		viewport.WorkPos.y = viewport.WorkSize.y * 2;
		ImGui::SetNextWindowPos(viewport.WorkPos);
		ImGui::SetNextWindowSize(viewport.WorkSize);
		ImGui::Begin("URLs", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			urls.drawGui();
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
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF); // See: https://stackoverflow.com/questions/30413066/how-do-i-diagnose-heap-corruption-errors-on-windows
	HWND hWnd = GetConsoleWindow();
	FreeConsole();
	MyGame* mg = new MyGame();
	mg->start(1280, 720, "M.O.THE.R - Memory of the repetitive");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
