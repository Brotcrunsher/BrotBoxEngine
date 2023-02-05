#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <ctime>
#include <chrono>

class MyGame : public bbe::Game
{
	std::chrono::system_clock::time_point targetTime;
	bbe::Font font;
	bbe::String milliseconds;
	bbe::String seconds;
	bbe::String minutes;
	bbe::String hours;

	float timeSinceTimeUp = 0;
	const int32_t widthOfChars = 85;

	virtual void onStart() override
	{
		std::tm targetTimeVerbose{};
		targetTimeVerbose.tm_sec = 0;   // seconds after the minute - [0, 60] including leap second
		targetTimeVerbose.tm_min = 0;   // minutes after the hour - [0, 59]
		targetTimeVerbose.tm_hour = 14;  // hours since midnight - [0, 23]
		targetTimeVerbose.tm_mday = 12;  // day of the month - [1, 31]
		targetTimeVerbose.tm_mon = 6;   // months since January - [0, 11]
		targetTimeVerbose.tm_year = 120;  // years since 1900

		targetTime = std::chrono::system_clock::from_time_t(std::mktime(&targetTimeVerbose));

		font.load("arial.ttf", 180);
		font.setFixedWidth(widthOfChars);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		auto now = std::chrono::system_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(targetTime - now).count();
		if (diff < 0)
		{
			timeSinceTimeUp += timeSinceLastFrame;
			diff = 0;
		}
		milliseconds = bbe::String(diff % 1000).leftFill('0', 3);
		diff /= 1000;
		seconds = bbe::String(diff % 60).leftFill('0', 2);
		diff /= 60;
		minutes = bbe::String(diff % 60).leftFill('0', 2);
		diff /= 60;
		hours = bbe::String(diff).leftFill('0', 2);
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		if (timeSinceTimeUp > 0)
		{
			brush.setColorHSV(timeSinceTimeUp * 90, 1, 1);
		}
		float stringOffsetX = 120;
		float stringOffsetY = 400;
		brush.fillText(stringOffsetX,                      stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.1) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), hours       .getRaw(), font);
		brush.fillText(stringOffsetX + widthOfChars * 2.4, stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.2) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), ":", font);
		brush.fillText(stringOffsetX + widthOfChars * 3,   stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.3) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), minutes     .getRaw(), font);
		brush.fillText(stringOffsetX + widthOfChars * 5.4, stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.4) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), ":", font);
		brush.fillText(stringOffsetX + widthOfChars * 6,   stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.5) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), seconds     .getRaw(), font);
		brush.fillText(stringOffsetX + widthOfChars * 8.4, stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.6) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), ":", font);
		brush.fillText(stringOffsetX + widthOfChars * 9,   stringOffsetY + bbe::Math::sin(timeSinceTimeUp + 0.7) * bbe::Math::min(timeSinceTimeUp * 10, 100.0f), milliseconds.getRaw(), font);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "Countdown!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

