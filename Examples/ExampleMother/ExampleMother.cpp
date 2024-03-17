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
#include "EMTask.h"
#include "EMProcess.h"
#include "EMUrl.h"
#include "EMTab.h"
#include "EMBrainTeaser.h"

//TODO: Redo
//TODO: Countdown beeps when starting and stopping startable tasks
//TODO: Gamification, add a score how much time I needed to do all Now Tasks
//TODO: Clipboard: It should be possible to add a title
//TODO: Bug: When switching headphones, the sound system doesn't switch as well. It stays playing sounds on the old device.
//TODO: bbe::String should have bbe::List<char>

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

struct RememberList
{
	bbe::String title;
	bbe::List<bbe::String> entries;

	// Non-Persisted Helper Data below.
	bbe::String newEntryBuffer;

	void serialize(bbe::ByteBuffer& buffer) const
	{
		buffer.write(title);
		// TODO: A buffer.write(bbe::List<T>::serialize) would be useful.
		buffer.write((uint64_t)entries.getLength());
		for (size_t i = 0; i < entries.getLength(); i++)
		{
			buffer.write(entries[i]);
		}
	}
	static RememberList deserialize(bbe::ByteBufferSpan& buffer)
	{
		RememberList retVal;

		buffer.read(retVal.title);
		uint64_t entries = 0;
		buffer.read(entries);
		for (size_t i = 0; i < entries; i++)
		{
			// TODO: A returning read function would simplify this.
			bbe::String entry;
			buffer.read(entry);
			retVal.entries.add(entry);
		}

		return retVal;
	}
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
	bbe::SerializableList<RememberList> rememberLists           = bbe::SerializableList<RememberList>     ("RemeberLists.dat",        "ParanoiaConfig");
	bbe::SerializableObject<KeyboardTracker> keyboardTracker    = bbe::SerializableObject<KeyboardTracker>("keyboardTracker.dat"); // No ParanoiaConfig to avoid accidentally logging passwords.

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
		if (daysSinceQualifyingDate > 180) daysSinceQualifyingDate = 180;

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
		processes.update();

		beginMeasure("URL Stuff");
		urls.update();

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
			bool shouldPlayOpenTasks = !openTasksNotificationSilencedProcess && processes.isGameOn();

			// ... because urls.
			shouldPlayOpenTasks |= !openTasksNotificationSilencedUrl && urls.timeWasterFound();
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
		static float multiplier = 1.0f;
		ImGui::SliderFloat("Multiplier", &multiplier, 0.0f, 1.0f);

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
					grid[x][y] *= multiplier;
					image.setPixel(x, y, bbe::Color(grid[x][y] > 1.f ? 1.f : grid[x][y]));
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
					ImGui::Text("%s", rememberLists[i].entries[k].getRaw());
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
				Tab{"VTasks",    "View Tasks",     [&]() { return tasks.drawTabViewTasks(); }},
				Tab{"ETasks",    "Edit Tasks",     [&]() { return tasks.drawTabEditTasks(); }},
				Tab{"Clpbrd",    "Clipboard",      [&]() { return drawTabClipboard(); }},
				Tab{"Brn-T",     "Brain-Teaser",   [&]() { return brainTeasers.drawTabBrainTeasers(brush); }},
				Tab{"Stpwtch",   "Stopwatch",      [&]() { return drawTabStopwatch(); }},
				Tab{"MsTrck",    "Mouse Track",    [&]() { return drawTabMouseTracking(brush); }},
				Tab{"KybrdTrck", "Keyboard Track", [&]() { return drawTabKeyboardTracking(brush); }},
				Tab{"Terri",     "Territorial",    [&]() { return drawTabTerri(brush); }},
				Tab{"Lsts",      "Lists",          [&]() { return drawTabRememberLists(); }},
				Tab{"Cnsl",      "Console",        [&]() { return drawTabConsole(); }},
				Tab{"Cnfg",      "Config",         [&]() { return drawTabConfig(); }},
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
	HWND hWnd = GetConsoleWindow(); 
	FreeConsole();
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "M.O.THE.R - Memory of the repetitive");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}