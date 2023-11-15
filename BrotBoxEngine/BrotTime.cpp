#include "BBE/BrotTime.h"

bbe::TimePoint::TimePoint() :
	m_time(std::chrono::system_clock::now())
{
}

bbe::TimePoint::TimePoint(const std::chrono::system_clock::time_point &time) :
	m_time(time)
{
}

bbe::TimePoint::TimePoint(std::time_t time) :
	m_time(std::chrono::system_clock::from_time_t(time))
{
}

bbe::TimePoint bbe::TimePoint::nextMorning(int64_t morningHour) const
{
	std::time_t t = std::chrono::system_clock::to_time_t(m_time);
	::tm timeinfo;
	::localtime_s(&timeinfo, &t);
	if (timeinfo.tm_hour >= morningHour)
	{
		bbe::TimePoint nextDay(m_time);
		nextDay = nextDay.plusDays(1);
		t = std::chrono::system_clock::to_time_t(nextDay.m_time);
		::localtime_s(&timeinfo, &t);
	}
	timeinfo.tm_sec = 0;
	timeinfo.tm_min = 0;
	timeinfo.tm_hour = morningHour;
	t = ::mktime(&timeinfo);
	return TimePoint(t);
}

bbe::TimePoint bbe::TimePoint::plusDays(int64_t days) const
{
	using namespace std::literals;
	return m_time + 24h * days;
}

bbe::TimePoint bbe::TimePoint::plusHours(int64_t hours) const
{
	using namespace std::literals;
	return m_time + 1h * hours;
}

bbe::TimePoint bbe::TimePoint::plusMinutes(int64_t minutes) const
{
	using namespace std::literals;
	return m_time + 1min * minutes;
}

bbe::TimePoint bbe::TimePoint::plusSeconds(int64_t seconds) const
{
	using namespace std::literals;
	return m_time + 1s * seconds;
}

bbe::TimePoint bbe::TimePoint::plusMilliseconds(int64_t ms) const
{
	using namespace std::literals;
	return m_time + 1ms * ms;
}

bbe::Duration bbe::TimePoint::operator-(const bbe::TimePoint& other) const
{
	return bbe::Duration(m_time - other.m_time);
}

bool bbe::TimePoint::operator<(const bbe::TimePoint& other) const
{
	return m_time < other.m_time;
}

bool bbe::TimePoint::hasPassed() const
{
	// Rationale for >= instead of ==:
	// Taking the current time does itself always take a tiny bit of time (maybe sub nano seconds but not 0).
	// So when ever you take the current time, that meassurement must have already passed the moment we even
	// look at it. It can only be == because the measurement isn't precise enough. In other words,
	// TimePoint().hasPassed() shall always return true. Additionally, this assumption does simplify some code
	// in ExampleMother, which may indicate that it's the right choice for other situations as well.
	return std::chrono::system_clock::now() >= m_time;
}

bool bbe::TimePoint::isNight(int64_t fromHour, int64_t toHour) const
{
	std::time_t t = std::chrono::system_clock::to_time_t(m_time);
	::tm timeinfo;
	::localtime_s(&timeinfo, &t);

	if (fromHour < toHour)
	{
		return timeinfo.tm_hour >= fromHour && timeinfo.tm_hour <= toHour;
	}
	else
	{
		return timeinfo.tm_hour >= fromHour || timeinfo.tm_hour <= toHour;
	}
}

bool bbe::TimePoint::isSunday() const
{
	std::time_t t = std::chrono::system_clock::to_time_t(m_time);
	::tm timeinfo;
	::localtime_s(&timeinfo, &t);
	return timeinfo.tm_wday == 0;
}

bool bbe::TimePoint::isToday() const
{
	TimePoint now;

	std::time_t thisT = std::chrono::system_clock::to_time_t(m_time);
	std::time_t nowT = std::chrono::system_clock::to_time_t(now.m_time);

	::tm thisTimeinfo;
	::tm nowTimeinfo;
	::localtime_s(&thisTimeinfo, &thisT);
	::localtime_s(&nowTimeinfo, &nowT);

	return thisTimeinfo.tm_year == nowTimeinfo.tm_year && thisTimeinfo.tm_yday == nowTimeinfo.tm_yday;
}

bbe::String bbe::TimePoint::toString() const
{
	::time_t t = std::chrono::system_clock::to_time_t(m_time);
	char* c = ::ctime(&t);
	if (!c) c = "ERROR";
	return bbe::String(c);
}

void bbe::TimePoint::serialize(bbe::ByteBuffer& buffer) const
{
	int64_t val = (int64_t)std::chrono::system_clock::to_time_t(m_time);
	buffer.write(val);
}

bbe::TimePoint bbe::TimePoint::deserialize(bbe::ByteBufferSpan& buffer)
{
	int64_t val;
	buffer.read(val);
	return TimePoint((::time_t)val);
}

bbe::Duration::Duration()
{
}

bbe::Duration::Duration(const std::chrono::system_clock::duration& duration) :
	m_duration(duration)
{
}

bbe::String bbe::Duration::toString() const
{
	int32_t seconds = std::chrono::duration_cast<std::chrono::seconds>(m_duration).count();
	int32_t minutes = seconds / 60;
	seconds %= 60;
	int32_t hours = minutes / 60;
	minutes %= 60;

	// TODO: bbe::String::format would be nice.

	char buffer[128] = {};
	std::snprintf(buffer, sizeof(buffer), "%.2d:%.2d:%.2d", hours, minutes, seconds);
	return bbe::String(buffer);
}
