#pragma once

#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): Serializable macro + time types.

#include <functional>

struct GoogleCalendarParsedEvent
{
	bbe::String id;
	bbe::String title;
	bbe::TimePoint start{};
	bool allDay = false;
};

struct GoogleCalendarConfig
{
	BBE_SERIALIZABLE_DATA(
		((bbe::String), oauthClientId),
		((bbe::String), calendarId),
		((bbe::String), refreshToken),
		((bbe::String), accessToken),
		((bbe::TimePoint), accessTokenExpiry, bbe::TimePoint::epoch()),
		((bbe::String), oauthClientSecret))
};

namespace emGoogleCalendar
{
	void openBrowserToUrl(const bbe::String &url);

#if defined(BBE_ADD_CURL)
	/** Run OAuth loopback flow on a worker thread; returns immediately. */
	void startOAuthLoopbackAsync(const bbe::String &clientId, const bbe::String &clientSecret, std::function<void(bool success, const bbe::String &refreshToken, const bbe::String &error)> onComplete);

	bool refreshAccessToken(GoogleCalendarConfig &cfg, bbe::String &errorOut);
	bool fetchCalendarEvents(const GoogleCalendarConfig &cfg, bbe::TimePoint timeMin, bbe::TimePoint timeMax, bbe::List<GoogleCalendarParsedEvent> &out, bbe::String &errorOut);
	bool parseEventsJson(const bbe::String &jsonUtf8, bbe::List<GoogleCalendarParsedEvent> &out, bbe::String &errorOut);
#else
	inline void startOAuthLoopbackAsync(const bbe::String &, const bbe::String &, std::function<void(bool, const bbe::String &, const bbe::String &)> onComplete)
	{
		onComplete(false, {}, "curl support disabled (BBE_ADD_CURL)");
	}
	inline bool refreshAccessToken(GoogleCalendarConfig &, bbe::String &errorOut)
	{
		errorOut = "curl support disabled (BBE_ADD_CURL)";
		return false;
	}
	inline bool fetchCalendarEvents(const GoogleCalendarConfig &, bbe::TimePoint, bbe::TimePoint, bbe::List<GoogleCalendarParsedEvent> &, bbe::String &errorOut)
	{
		errorOut = "curl support disabled (BBE_ADD_CURL)";
		return false;
	}
	inline bool parseEventsJson(const bbe::String &, bbe::List<GoogleCalendarParsedEvent> &, bbe::String &errorOut)
	{
		errorOut = "curl support disabled (BBE_ADD_CURL)";
		return false;
	}
#endif
} // namespace emGoogleCalendar
