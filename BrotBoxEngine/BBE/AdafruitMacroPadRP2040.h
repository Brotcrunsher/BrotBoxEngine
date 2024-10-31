#pragma once
#ifdef WIN32

#include <thread>
#include <atomic>
#include <mutex>
#include <array>

#include "../BBE/List.h"

typedef void* HANDLE;

namespace bbe
{
	enum class RP2040Key
	{
		BUTTON_0  = 0,
		BUTTON_1  = 1,
		BUTTON_2  = 2,
		BUTTON_3  = 3,
		BUTTON_4  = 4,
		BUTTON_5  = 5,
		BUTTON_6  = 6,
		BUTTON_7  = 7,
		BUTTON_8  = 8,
		BUTTON_9  = 9,
		BUTTON_10 = 10,
		BUTTON_11 = 11,
		
		BUTTON_ROTATE_AUDIO = 100,
		BUTTON_AUDIO = 101,

		BUTTON_LAST = BUTTON_AUDIO
	};

	class AdafruitMacroPadRP2040
	{
	private:
		std::atomic_bool threadRunning = false;

		struct KeyEvent
		{
			unsigned char key = 0;
			unsigned char pressed = 0;
		};
		std::mutex keyEventsMutex;
		bbe::List<KeyEvent> keyEvents;
		HANDLE winHandle = nullptr;
		HANDLE stopEvent = nullptr;

		std::jthread thread;
		void threadMain();

		std::array<bool, (size_t)RP2040Key::BUTTON_LAST + 1> keyDown = {};
		std::array<bool, (size_t)RP2040Key::BUTTON_LAST + 1> keyPressed = {};
		bool anyActivityFound = false;

		int64_t rotationValue = 0;

	public:
		AdafruitMacroPadRP2040();
		~AdafruitMacroPadRP2040();

		AdafruitMacroPadRP2040(const AdafruitMacroPadRP2040&) = delete;
		AdafruitMacroPadRP2040(AdafruitMacroPadRP2040&&) = delete;
		AdafruitMacroPadRP2040& operator=(const AdafruitMacroPadRP2040&) = delete;
		AdafruitMacroPadRP2040& operator=(AdafruitMacroPadRP2040&&) = delete;

		bool connect();
		void disconnect();

		bool isConnected() const;
		bool anyActivity() const;

		bool isKeyDown(RP2040Key key) const;
		bool isKeyUp(RP2040Key key) const;
		bool isKeyPressed(RP2040Key key) const;
		int64_t getRotationValue() const;

		void update();
	};
}
#endif
