#pragma once

#include <chrono>
#include "../BBE/ByteBuffer.h"
#include "../BBE/String.h"

namespace bbe
{
	class Duration
	{
	private:
		std::chrono::system_clock::duration m_duration = std::chrono::system_clock::duration::zero();

	public:
		Duration();
		explicit Duration(const std::chrono::system_clock::duration& duration);

		static Duration fromMilliseconds(int64_t millis);

		auto operator<=>(const Duration&) const = default;

		bbe::String toString(bool showMillis = false) const;
		int64_t toMillis() const;
		int64_t toSeconds() const;
		int32_t toMinutes() const;
		int32_t toHours() const;
		int32_t toDays() const;
		bool isNegative() const;
	};

	enum class Month
	{
		JANUARY   = 1,
		FEBRUARY  = 2,
		MARCH     = 3,
		APRIL     = 4,
		MAY       = 5,
		JUNE      = 6,
		JULY      = 7,
		AUGUST    = 8,
		SEPTEMBER = 9,
		OCTOBER   = 10,
		NOVEMBER  = 11,
		DECEMBER  = 12,
	};

	enum class Weekday
	{
		MONDAY    = 0,
		TUESDAY   = 1,
		WEDNESDAY = 2,
		THURSDAY  = 3,
		FRIDAY    = 4,
		SATURDAY  = 5,
		SUNDAY    = 6,
	};

	class TimePoint
	{
	private:
		std::chrono::system_clock::time_point m_time;

		::tm toTm() const;

	public:
		TimePoint();
		explicit TimePoint(const std::chrono::system_clock::time_point &time);
		explicit TimePoint(std::time_t time);

		static TimePoint todayAt(int32_t hour, int32_t minute, int32_t second = 0);
		static TimePoint fromDate(int32_t year, Month   month, int32_t day, int32_t hour = 0, int32_t minute = 0, int32_t second = 0);
		static TimePoint fromDate(int32_t year, int32_t month, int32_t day, int32_t hour = 0, int32_t minute = 0, int32_t second = 0);
		static TimePoint epoch();

		static int32_t isLeapYear(int32_t year);
		static int32_t getDaysInMonth(int32_t year, Month month);
		static int32_t getDaysInMonth(int32_t year, int32_t month);
		static Weekday getFirstWeekdayOfMonth(int32_t year, Month month);

		TimePoint nextMorning(int32_t morningHour = 5) const;
		TimePoint toMorning(int32_t morningHour = 5) const;
		TimePoint plusDays(int64_t days) const;
		TimePoint plusHours(int64_t hours) const;
		TimePoint plusMinutes(int64_t minutes) const;
		TimePoint plusSeconds(int64_t seconds) const;
		TimePoint plusMilliseconds(int64_t ms) const;

		Duration operator-(const TimePoint& other) const;

		bool operator<(const TimePoint& other) const;
		bool operator>(const TimePoint& other) const;
		bool operator==(const TimePoint& other) const;
		bool operator!=(const TimePoint& other) const;

		bool hasPassed() const;

		// Both hours are inclusive! So 23/4 is from 23:00 until 4:59.
		bool isNight(int64_t fromHour = 23, int64_t toHour = 4) const;
		Weekday getWeekday() const;
		bool isMonday() const;
		bool isTuesday() const;
		bool isWednesday() const;
		bool isThursday() const;
		bool isFriday() const;
		bool isSaturday() const;
		bool isSunday() const;
		bool isToday() const;
		bool isSameDay(const bbe::TimePoint& other) const;

		bbe::String toString() const;

		void serialize(bbe::ByteBuffer& buffer) const;
		static TimePoint deserialize(bbe::ByteBufferSpan& buffer);

		static TimePoint fromString(const bbe::String& s, const bbe::String& format);

		int32_t getYear() const;
		Month getMonth() const;
		int32_t getDay() const;
		int32_t getHour() const;
		int32_t getMinute() const;
		int32_t getSecond() const;

		int64_t toMilliseconds() const;
	};

	class TimeGate
	{
	private:
		TimePoint m_nextExec;

	public:
		TimeGate() = default;

		bool everyMilliseconds(int32_t ms);
		bool everySeconds(int32_t seconds);
		bool everyMinutes(int32_t minutes);
		bool everyHours(int32_t hours);

		static inline thread_local TimeGate* INTERNAL_INSPECTED_TIMEGATE = nullptr;
	};
#define EVERY_MILLISECONDS(ms) { static bbe::TimeGate tg; bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE = &tg; } if(bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE->everyMilliseconds((ms)))
#define EVERY_SECONDS(sec)     { static bbe::TimeGate tg; bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE = &tg; } if(bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE->everySeconds((sec)))
#define EVERY_MINUTES(min)     { static bbe::TimeGate tg; bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE = &tg; } if(bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE->everyMinutes((min)))
#define EVERY_HOURS(hrs)       { static bbe::TimeGate tg; bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE = &tg; } if(bbe::TimeGate::INTERNAL_INSPECTED_TIMEGATE->everyHours((hrs)))
}
