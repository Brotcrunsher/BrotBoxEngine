#include "EMGoogleCalendar.h"

#if defined(BBE_ADD_CURL)

#include "BBE/SimpleUrlRequest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <ctime>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include <sodium.h>
#include <curl/curl.h>

#ifdef _WIN32
#define NOMINMAX
#include <shellapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#endif

namespace emGoogleCalendar
{
	namespace
	{
		bbe::String urlEncodeCurl(CURL *curl, const bbe::String &s)
		{
			char *esc = curl_easy_escape(curl, s.getRaw(), (int)s.getLengthBytes());
			if (!esc)
				return s;
			bbe::String out(esc);
			curl_free(esc);
			return out;
		}

		bbe::String formatRfc3339Utc(const bbe::TimePoint &tp)
		{
			const int64_t ms = tp.toMilliseconds();
			const time_t sec = (time_t)(ms / 1000);
#ifdef _WIN32
			struct tm t
			{
			};
			gmtime_s(&t, &sec);
#else
			struct tm t
			{
			};
			gmtime_r(&sec, &t);
#endif
			char buf[48] = {};
			strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
			return bbe::String(buf);
		}

		bool parseIsoStart(const std::string &s, bbe::TimePoint &out, bbe::String &err)
		{
			if (s.size() >= 10 && s.find('T') == std::string::npos)
			{
				int y = 0;
				int M = 0;
				int d = 0;
				if (sscanf(s.c_str(), "%d-%d-%d", &y, &M, &d) != 3)
				{
					err = "Invalid all-day date";
					return false;
				}
				out = bbe::TimePoint::fromDate(y, M, d, 8, 0, 0);
				return true;
			}
			if (s.size() < 19)
			{
				err = "dateTime too short";
				return false;
			}
			int y = 0;
			int M = 0;
			int d = 0;
			int h = 0;
			int mi = 0;
			int se = 0;
			if (sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &y, &M, &d, &h, &mi, &se) < 6)
			{
				err = "Invalid dateTime prefix";
				return false;
			}
			size_t i = 19;
			while (i < s.size() && s[i] == '.')
			{
				i++;
				while (i < s.size() && s[i] >= '0' && s[i] <= '9')
					i++;
			}
			if (i >= s.size())
			{
				err = "Missing timezone in dateTime";
				return false;
			}
			if (s[i] == 'Z')
			{
				struct tm t
				{
				};
				memset(&t, 0, sizeof(t));
				t.tm_year = y - 1900;
				t.tm_mon = M - 1;
				t.tm_mday = d;
				t.tm_hour = h;
				t.tm_min = mi;
				t.tm_sec = se;
#ifdef _WIN32
				const time_t tt = _mkgmtime(&t);
#else
				const time_t tt = timegm(&t);
#endif
				if (tt == (time_t)-1)
				{
					err = "timegm failed";
					return false;
				}
				out = bbe::TimePoint(tt);
				return true;
			}
			if (s[i] != '+' && s[i] != '-')
			{
				err = "Unsupported dateTime suffix";
				return false;
			}
			const char sign = s[i];
			int oh = 0;
			int om = 0;
			if (sscanf(s.c_str() + i + 1, "%d:%d", &oh, &om) != 2)
			{
				err = "Bad offset";
				return false;
			}
			struct tm t
			{
			};
			memset(&t, 0, sizeof(t));
			t.tm_year = y - 1900;
			t.tm_mon = M - 1;
			t.tm_mday = d;
			t.tm_hour = h;
			t.tm_min = mi;
			t.tm_sec = se;
#ifdef _WIN32
			const time_t naiveUtc = _mkgmtime(&t);
#else
			const time_t naiveUtc = timegm(&t);
#endif
			if (naiveUtc == (time_t)-1)
			{
				err = "timegm (civil) failed";
				return false;
			}
			// ISO 8601 offset: local_instant = UTC + offset (+02 means east). UTC = naiveUtc - offset.
			const int isoOffsetSec = (sign == '+' ? 1 : -1) * (oh * 3600 + om * 60);
			const time_t utc = naiveUtc - isoOffsetSec;
			out = bbe::TimePoint(utc);
			return true;
		}

		int openListeningSocket(uint16_t &outPort)
		{
#ifdef _WIN32
			SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (s == INVALID_SOCKET)
				return -1;
			BOOL opt = TRUE;
			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			addr.sin_port = 0;
			if (bind(s, (sockaddr *)&addr, sizeof(addr)) != 0)
			{
				closesocket(s);
				return -1;
			}
			int namelen = (int)sizeof(addr);
			if (getsockname(s, (sockaddr *)&addr, &namelen) != 0)
			{
				closesocket(s);
				return -1;
			}
			outPort = ntohs(addr.sin_port);
			if (listen(s, 1) != 0)
			{
				closesocket(s);
				return -1;
			}
			return (int)(intptr_t)s;
#else
			const int s = socket(AF_INET, SOCK_STREAM, 0);
			if (s < 0)
				return -1;
			int opt = 1;
			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			addr.sin_port = 0;
			if (bind(s, (sockaddr *)&addr, sizeof(addr)) != 0)
			{
				close(s);
				return -1;
			}
			socklen_t len = sizeof(addr);
			if (getsockname(s, (sockaddr *)&addr, &len) != 0)
			{
				close(s);
				return -1;
			}
			outPort = ntohs(addr.sin_port);
			if (listen(s, 1) != 0)
			{
				close(s);
				return -1;
			}
			return s;
#endif
		}

		void closeSocket(int s)
		{
			if (s < 0)
				return;
#ifdef _WIN32
			closesocket((SOCKET)s);
#else
			close(s);
#endif
		}

		bool recvHttpExtractCode(int clientSock, bbe::String &outCode, bbe::String &err)
		{
			std::string buf;
			buf.resize(8192);
			size_t total = 0;
			while (total < buf.size() - 1)
			{
#ifdef _WIN32
				const int n = recv((SOCKET)clientSock, buf.data() + total, (int)(buf.size() - 1 - total), 0);
#else
				const ssize_t n = recv(clientSock, buf.data() + total, buf.size() - 1 - total, 0);
#endif
				if (n <= 0)
					break;
				total += (size_t)n;
				buf[total] = 0;
				if (strstr(buf.c_str(), "\r\n\r\n"))
					break;
			}
			buf.resize(total);
			const char *q = strstr(buf.c_str(), "code=");
			if (!q)
			{
				err = "No OAuth code in callback";
				return false;
			}
			q += 5;
			const char *amp = strchr(q, '&');
			const char *sp = strchr(q, ' ');
			const char *end = buf.c_str() + buf.size();
			const char *e = end;
			if (amp && amp < e)
				e = amp;
			if (sp && sp < e)
				e = sp;
			outCode = bbe::String(std::string(q, e).c_str());
			return !outCode.isEmpty();
		}

		void sendBrowserOk(int clientSock)
		{
			const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n"
							   "<html><body><p>Google Calendar connected. You can close this tab.</p></body></html>\r\n";
#ifdef _WIN32
			send((SOCKET)clientSock, resp, (int)strlen(resp), 0);
#else
			(void)send(clientSock, resp, strlen(resp), 0);
#endif
		}

		bbe::String randomPkceVerifier()
		{
			unsigned char rnd[32];
			randombytes_buf(rnd, sizeof(rnd));
			std::vector<char> out(sodium_base64_encoded_len(sizeof(rnd), sodium_base64_VARIANT_URLSAFE_NO_PADDING));
			sodium_bin2base64(out.data(), out.size(), rnd, sizeof(rnd), sodium_base64_VARIANT_URLSAFE_NO_PADDING);
			return bbe::String(out.data());
		}

		bbe::String pkceChallengeS256(const bbe::String &verifier)
		{
			unsigned char hash[crypto_hash_sha256_BYTES];
			crypto_hash_sha256(hash, (const unsigned char *)verifier.getRaw(), verifier.getLengthBytes());
			std::vector<char> out(sodium_base64_encoded_len(sizeof(hash), sodium_base64_VARIANT_URLSAFE_NO_PADDING));
			sodium_bin2base64(out.data(), out.size(), hash, sizeof(hash), sodium_base64_VARIANT_URLSAFE_NO_PADDING);
			return bbe::String(out.data());
		}

		bool exchangeCodeForTokens(const bbe::String &clientId, const bbe::String &clientSecret, const bbe::String &redirectUri, const bbe::String &code, const bbe::String &codeVerifier, bbe::String &refreshOut, bbe::String &errorOut)
		{
			CURL *curl = curl_easy_init();
			if (!curl)
			{
				errorOut = "curl_easy_init failed";
				return false;
			}
			bbe::String body = bbe::String("client_id=") + urlEncodeCurl(curl, clientId) + "&grant_type=authorization_code&code=" + urlEncodeCurl(curl, code) + "&redirect_uri=" + urlEncodeCurl(curl, redirectUri) + "&code_verifier=" + urlEncodeCurl(curl, codeVerifier);
			if (!clientSecret.isEmpty())
				body += bbe::String("&client_secret=") + urlEncodeCurl(curl, clientSecret);
			const bbe::List<bbe::String> headers = { "Content-Type: application/x-www-form-urlencoded" };
			const auto res = bbe::simpleUrlRequest::urlRequest("https://oauth2.googleapis.com/token", headers, body, true, false);
			curl_easy_cleanup(curl);
			if (res.responseCode != 200)
			{
				errorOut = bbe::String::format("Token HTTP %ld: %s", res.responseCode, res.dataContainer.getRaw());
				return false;
			}
			try
			{
				const nlohmann::json j = nlohmann::json::parse(res.dataContainer.getRaw());
				if (!j.contains("refresh_token"))
				{
					errorOut = "No refresh_token (try revoking app access in Google account and reconnect with prompt=consent)";
					return false;
				}
				refreshOut = j["refresh_token"].get<std::string>().c_str();
				return true;
			}
			catch (const std::exception &e)
			{
				errorOut = e.what();
				return false;
			}
		}
	} // namespace

	void openBrowserToUrl(const bbe::String &url)
	{
#ifdef _WIN32
		ShellExecuteA(nullptr, "open", url.getRaw(), nullptr, nullptr, SW_SHOWNORMAL);
#else
		const pid_t pid = fork();
		if (pid == 0)
		{
			execlp("xdg-open", "xdg-open", url.getRaw(), (char *)nullptr);
			_exit(127);
		}
#endif
	}

	void startOAuthLoopbackAsync(const bbe::String &clientId, const bbe::String &clientSecret, std::function<void(bool success, const bbe::String &refreshToken, const bbe::String &error)> onComplete)
	{
		std::thread([clientId, clientSecret, onComplete]() {
			if (sodium_init() < 0)
			{
				onComplete(false, {}, "sodium_init failed");
				return;
			}
#ifdef _WIN32
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			{
				onComplete(false, {}, "WSAStartup failed");
				return;
			}
#endif
			uint16_t port = 0;
			const int srv = openListeningSocket(port);
			if (srv < 0)
			{
				onComplete(false, {}, "Could not bind loopback socket");
#ifdef _WIN32
				WSACleanup();
#endif
				return;
			}
			const bbe::String redirectUri = bbe::String::format("http://127.0.0.1:%u/", (unsigned)port);
			const bbe::String verifier = randomPkceVerifier();
			const bbe::String challenge = pkceChallengeS256(verifier);
			CURL *curl = curl_easy_init();
			const bbe::String encClient = curl ? urlEncodeCurl(curl, clientId) : clientId;
			const bbe::String encRedir = curl ? urlEncodeCurl(curl, redirectUri) : redirectUri;
			const bbe::String encScope = curl ? urlEncodeCurl(curl, "https://www.googleapis.com/auth/calendar.readonly") : bbe::String("https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fcalendar.readonly");
			const bbe::String encCh = curl ? urlEncodeCurl(curl, challenge) : challenge;
			if (curl)
				curl_easy_cleanup(curl);
			const bbe::String authUrl = bbe::String("https://accounts.google.com/o/oauth2/v2/auth?client_id=") + encClient + "&redirect_uri=" + encRedir + "&response_type=code&scope=" + encScope + "&code_challenge=" + encCh + "&code_challenge_method=S256&access_type=offline&prompt=consent";
			openBrowserToUrl(authUrl);

			bool ok = false;
			bbe::String code;
			bbe::String err;
#ifdef _WIN32
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET((SOCKET)srv, &rfds);
			struct timeval tv;
			tv.tv_sec = 300;
			tv.tv_usec = 0;
			const int sel = select(0, &rfds, nullptr, nullptr, &tv);
			if (sel <= 0)
				err = "OAuth wait timeout";
			else
			{
				const SOCKET c = accept((SOCKET)srv, nullptr, nullptr);
				if (c == INVALID_SOCKET)
					err = "accept failed";
				else
				{
					if (!recvHttpExtractCode((int)c, code, err))
					{
					}
					else
					{
						sendBrowserOk((int)c);
						ok = true;
					}
					closesocket(c);
				}
			}
#else
			struct pollfd pfd;
			pfd.fd = srv;
			pfd.events = POLLIN;
			const int pr = poll(&pfd, 1, 300000);
			if (pr <= 0)
				err = "OAuth wait timeout";
			else
			{
				const int c = accept(srv, nullptr, nullptr);
				if (c < 0)
					err = "accept failed";
				else
				{
					if (!recvHttpExtractCode(c, code, err))
					{
					}
					else
					{
						sendBrowserOk(c);
						ok = true;
					}
					close(c);
				}
			}
#endif
			closeSocket(srv);
#ifdef _WIN32
			WSACleanup();
#endif
			if (!ok || code.isEmpty())
			{
				onComplete(false, {}, err.isEmpty() ? bbe::String("OAuth failed") : err);
				return;
			}
			bbe::String refresh;
			if (!exchangeCodeForTokens(clientId, clientSecret, redirectUri, code, verifier, refresh, err))
			{
				onComplete(false, {}, err);
				return;
			}
			onComplete(true, refresh, {});
		}).detach();
	}

	bool refreshAccessToken(GoogleCalendarConfig &cfg, bbe::String &errorOut)
	{
		if (cfg.oauthClientId.isEmpty() || cfg.refreshToken.isEmpty())
		{
			errorOut = "Missing client id or refresh token";
			return false;
		}
		CURL *curl = curl_easy_init();
		if (!curl)
		{
			errorOut = "curl_easy_init failed";
			return false;
		}
		bbe::String body = bbe::String("client_id=") + urlEncodeCurl(curl, cfg.oauthClientId) + "&grant_type=refresh_token&refresh_token=" + urlEncodeCurl(curl, cfg.refreshToken);
		if (!cfg.oauthClientSecret.isEmpty())
			body += bbe::String("&client_secret=") + urlEncodeCurl(curl, cfg.oauthClientSecret);
		const bbe::List<bbe::String> headers = { "Content-Type: application/x-www-form-urlencoded" };
		const auto res = bbe::simpleUrlRequest::urlRequest("https://oauth2.googleapis.com/token", headers, body, true, false);
		curl_easy_cleanup(curl);
		if (res.responseCode != 200)
		{
			errorOut = bbe::String::format("Refresh HTTP %ld: %s", res.responseCode, res.dataContainer.getRaw());
			return false;
		}
		try
		{
			const nlohmann::json j = nlohmann::json::parse(res.dataContainer.getRaw());
			cfg.accessToken = j["access_token"].get<std::string>().c_str();
			int64_t expSec = 3600;
			if (j.contains("expires_in"))
				expSec = j["expires_in"].get<int64_t>();
			cfg.accessTokenExpiry = bbe::TimePoint().plusSeconds(expSec - 120);
			return true;
		}
		catch (const std::exception &e)
		{
			errorOut = e.what();
			return false;
		}
	}

	bool fetchCalendarEvents(const GoogleCalendarConfig &cfg, bbe::TimePoint timeMin, bbe::TimePoint timeMax, bbe::List<GoogleCalendarParsedEvent> &out, bbe::String &errorOut)
	{
		out.clear();
		if (cfg.accessToken.isEmpty())
		{
			errorOut = "No access token";
			return false;
		}
		bbe::String cal = cfg.calendarId;
		if (cal.isEmpty())
			cal = "primary";
		CURL *curl = curl_easy_init();
		if (!curl)
		{
			errorOut = "curl_easy_init failed";
			return false;
		}
		const bbe::String calEnc = urlEncodeCurl(curl, cal);
		curl_easy_cleanup(curl);
		const bbe::String url = bbe::String("https://www.googleapis.com/calendar/v3/calendars/") + calEnc + "/events?singleEvents=true&orderBy=startTime&maxResults=250&timeMin=" + formatRfc3339Utc(timeMin) + "&timeMax=" + formatRfc3339Utc(timeMax);
		const bbe::List<bbe::String> headers = { bbe::String("Authorization: Bearer ") + cfg.accessToken };
		const auto res = bbe::simpleUrlRequest::urlRequest(url, headers, {}, true, false);
		if (res.responseCode != 200)
		{
			errorOut = bbe::String::format("Events HTTP %ld: %s", res.responseCode, res.dataContainer.getRaw());
			return false;
		}
		return parseEventsJson(bbe::String(res.dataContainer.getRaw()), out, errorOut);
	}

	bool parseEventsJson(const bbe::String &jsonUtf8, bbe::List<GoogleCalendarParsedEvent> &out, bbe::String &errorOut)
	{
		out.clear();
		try
		{
			const nlohmann::json j = nlohmann::json::parse(jsonUtf8.getRaw());
			if (!j.contains("items") || !j["items"].is_array())
				return true;
			for (const auto &item : j["items"])
			{
				if (item.contains("status") && item["status"].is_string() && item["status"].get<std::string>() == "cancelled")
					continue;
				if (!item.contains("id") || !item["id"].is_string())
					continue;
				GoogleCalendarParsedEvent ev;
				ev.id = item["id"].get<std::string>().c_str();
				if (item.contains("summary") && item["summary"].is_string())
					ev.title = item["summary"].get<std::string>().c_str();
				else
					ev.title = "(No title)";
				if (!item.contains("start"))
				{
					continue;
				}
				const auto &st = item["start"];
				bbe::String perr;
				if (st.contains("date") && st["date"].is_string())
				{
					ev.allDay = true;
					if (!parseIsoStart(st["date"].get<std::string>(), ev.start, perr))
						continue;
				}
				else if (st.contains("dateTime") && st["dateTime"].is_string())
				{
					ev.allDay = false;
					if (!parseIsoStart(st["dateTime"].get<std::string>(), ev.start, perr))
						continue;
				}
				else
					continue;
				out.add(ev);
			}
			return true;
		}
		catch (const std::exception &e)
		{
			errorOut = e.what();
			return false;
		}
	}
} // namespace emGoogleCalendar

#else // !BBE_ADD_CURL

namespace emGoogleCalendar
{
	void openBrowserToUrl(const bbe::String &)
	{
	}
} // namespace emGoogleCalendar

#endif
