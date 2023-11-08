#pragma once

#include <chrono>
#include "../BBE/ByteBuffer.h"
#include "../BBE/String.h"

namespace bbe
{
	class Duration
	{
	private:
		std::chrono::system_clock::duration m_duration;

	public:
		Duration();
		Duration(const std::chrono::system_clock::duration& duration);

		bbe::String toString() const;
	};

	class TimePoint
	{
	private:
		std::chrono::system_clock::time_point m_time;

	public:
		TimePoint();
		TimePoint(const std::chrono::system_clock::time_point &time);
		TimePoint(std::time_t time);

		TimePoint nextMorning(int64_t morningHour = 5) const;
		TimePoint plusDays(int64_t days) const;
		TimePoint plusHours(int64_t hours) const;
		TimePoint plusMinutes(int64_t minutes) const;
		TimePoint plusSeconds(int64_t seconds) const;
		TimePoint plusMilliseconds(int64_t ms) const;

		Duration operator-(const TimePoint& other) const;

		bool hasPassed() const;

		// Both hours are inclusive! So 23/4 is from 23:00 until 4:59.
		bool isNight(int64_t fromHour = 23, int64_t toHour = 4) const;
		bool isSunday() const;
		bool isToday() const;

		bbe::String toString() const;

		void serialize(bbe::ByteBuffer& buffer) const;
		static TimePoint deserialize(bbe::ByteBufferSpan& buffer);
	};
}
