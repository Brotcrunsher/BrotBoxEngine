#include "GitRunner.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace gitReview
{
#ifdef _WIN32
	// ── Windows helpers ──────────────────────────────────────────────

	static std::wstring utf8ToWide(const std::string &s)
	{
		if (s.empty())
			return {};
		const int needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
		if (needed <= 0)
			return {};
		std::wstring w(static_cast<size_t>(needed), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), needed);
		return w;
	}

	static std::wstring quoteArgW(const std::string &arg)
	{
		std::wstring w = utf8ToWide(arg);
		bool needsQuoting = arg.empty();
		for (char c : arg)
		{
			if (c == ' ' || c == '\t' || c == '"' || c == '\\')
			{
				needsQuoting = true;
				break;
			}
		}
		if (!needsQuoting)
			return w;

		std::wstring result = L"\"";
		int numBackslashes = 0;
		for (wchar_t c : w)
		{
			if (c == L'\\')
			{
				numBackslashes++;
			}
			else if (c == L'"')
			{
				result.append(static_cast<size_t>(numBackslashes + 1), L'\\');
				result += L'"';
				numBackslashes = 0;
			}
			else
			{
				result.append(static_cast<size_t>(numBackslashes), L'\\');
				numBackslashes = 0;
				result += c;
			}
		}
		result.append(static_cast<size_t>(numBackslashes), L'\\');
		result += L'"';
		return result;
	}

	static void drainHandle(HANDLE h, std::string &into)
	{
		char buf[4096];
		DWORD n = 0;
		while (ReadFile(h, buf, sizeof(buf), &n, nullptr) && n > 0)
			into.append(buf, static_cast<size_t>(n));
	}

	static GitRunResult runGitImpl(const std::string &repoRoot, const std::vector<std::string> &args, const std::string *stdinData)
	{
		GitRunResult result;

		std::wstring cmdLine = L"git -C ";
		cmdLine += quoteArgW(repoRoot);
		for (const auto &a : args)
		{
			cmdLine += L' ';
			cmdLine += quoteArgW(a);
		}

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		HANDLE hStdoutRead = nullptr, hStdoutWrite = nullptr;
		HANDLE hStderrRead = nullptr, hStderrWrite = nullptr;
		HANDLE hStdinRead = nullptr, hStdinWrite = nullptr;

		if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0) || !CreatePipe(&hStderrRead, &hStderrWrite, &sa, 0))
		{
			result.standardError = "CreatePipe failed.";
			return result;
		}

		const bool useStdin = stdinData != nullptr;
		if (useStdin)
		{
			if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0))
			{
				CloseHandle(hStdoutRead);
				CloseHandle(hStdoutWrite);
				CloseHandle(hStderrRead);
				CloseHandle(hStderrWrite);
				result.standardError = "CreatePipe for stdin failed.";
				return result;
			}
			SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);
		}

		SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(hStderrRead, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW si{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = hStdoutWrite;
		si.hStdError = hStderrWrite;
		si.hStdInput = useStdin ? hStdinRead : GetStdHandle(STD_INPUT_HANDLE);

		PROCESS_INFORMATION pi{};
		std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
		cmdBuf.push_back(L'\0');

		const BOOL ok = CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

		CloseHandle(hStdoutWrite);
		CloseHandle(hStderrWrite);
		if (useStdin)
			CloseHandle(hStdinRead);

		if (!ok)
		{
			CloseHandle(hStdoutRead);
			CloseHandle(hStderrRead);
			if (useStdin)
				CloseHandle(hStdinWrite);
			result.standardError = "Failed to start git (CreateProcessW).";
			return result;
		}

		if (useStdin)
		{
			DWORD written = 0;
			const char *data = stdinData->data();
			DWORD remaining = static_cast<DWORD>(stdinData->size());
			while (remaining > 0)
			{
				if (!WriteFile(hStdinWrite, data, remaining, &written, nullptr))
					break;
				data += written;
				remaining -= written;
			}
			CloseHandle(hStdinWrite);
		}

		std::thread errThread(drainHandle, hStderrRead, std::ref(result.standardError));
		drainHandle(hStdoutRead, result.standardOut);
		errThread.join();

		CloseHandle(hStdoutRead);
		CloseHandle(hStderrRead);

		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		result.exitCode = static_cast<int>(exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return result;
	}

#else
	// ── POSIX helpers ────────────────────────────────────────────────

	static GitRunResult runGitImpl(const std::string &repoRoot, const std::vector<std::string> &args, const std::string *stdinData)
	{
		GitRunResult result;

		std::array<int, 2> outPipe{};
		std::array<int, 2> errPipe{};
		std::array<int, 2> inPipe{};

		if (pipe(outPipe.data()) != 0 || pipe(errPipe.data()) != 0)
		{
			result.standardError = "pipe() failed.";
			return result;
		}

		const bool useStdin = stdinData != nullptr;
		if (useStdin)
		{
			if (pipe(inPipe.data()) != 0)
			{
				close(outPipe[0]);
				close(outPipe[1]);
				close(errPipe[0]);
				close(errPipe[1]);
				result.standardError = "pipe() for stdin failed.";
				return result;
			}
		}

		const pid_t pid = fork();
		if (pid < 0)
		{
			close(outPipe[0]);
			close(outPipe[1]);
			close(errPipe[0]);
			close(errPipe[1]);
			if (useStdin)
			{
				close(inPipe[0]);
				close(inPipe[1]);
			}
			result.standardError = "fork() failed.";
			return result;
		}

		if (pid == 0)
		{
			close(outPipe[0]);
			close(errPipe[0]);
			dup2(outPipe[1], STDOUT_FILENO);
			dup2(errPipe[1], STDERR_FILENO);
			close(outPipe[1]);
			close(errPipe[1]);

			if (useStdin)
			{
				close(inPipe[1]);
				dup2(inPipe[0], STDIN_FILENO);
				close(inPipe[0]);
			}

			std::vector<std::string> argvStorage;
			argvStorage.reserve(args.size() + 3);
			argvStorage.push_back("git");
			argvStorage.push_back("-C");
			argvStorage.push_back(repoRoot);
			for (const auto &a : args)
				argvStorage.push_back(a);

			std::vector<char *> argvPtrs;
			argvPtrs.reserve(argvStorage.size() + 1);
			for (auto &s : argvStorage)
				argvPtrs.push_back(s.data());
			argvPtrs.push_back(nullptr);

			execvp("git", argvPtrs.data());
			_exit(127);
		}

		// Parent: close write ends of child-facing pipes.
		close(outPipe[1]);
		close(errPipe[1]);

		// Write stdin data to child before draining output.
		// Commit messages are small enough to fit in the kernel pipe buffer
		// (~64 KB on Linux) so this won't deadlock in practice.
		if (useStdin)
		{
			close(inPipe[0]);
			const char *data = stdinData->data();
			size_t remaining = stdinData->size();
			while (remaining > 0)
			{
				const ssize_t n = write(inPipe[1], data, remaining);
				if (n <= 0)
					break;
				data += n;
				remaining -= static_cast<size_t>(n);
			}
			close(inPipe[1]);
		}

		// Drain stdout and stderr concurrently to prevent deadlock:
		// a thread handles stderr while the main thread handles stdout.
		auto drainFd = [](int fd, std::string &into) {
			char buf[4096];
			for (;;)
			{
				const ssize_t n = read(fd, buf, sizeof(buf));
				if (n <= 0)
					break;
				into.append(buf, static_cast<size_t>(n));
			}
			close(fd);
		};

		std::thread errThread(drainFd, errPipe[0], std::ref(result.standardError));
		drainFd(outPipe[0], result.standardOut);
		errThread.join();

		int status = 0;
		if (waitpid(pid, &status, 0) < 0)
		{
			result.exitCode = -1;
			return result;
		}
		if (WIFEXITED(status))
			result.exitCode = WEXITSTATUS(status);
		else
			result.exitCode = -1;
		return result;
	}
#endif

	// ── Public API ───────────────────────────────────────────────────

	GitRunResult runGit(const std::string &repoRoot, const std::vector<std::string> &args)
	{
		return runGitImpl(repoRoot, args, nullptr);
	}

	GitRunResult runGitWithStdin(const std::string &repoRoot, const std::vector<std::string> &args, const std::string &stdinData)
	{
		return runGitImpl(repoRoot, args, &stdinData);
	}

	std::string redactGitOutput(const std::string &text)
	{
		std::string out;
		out.reserve(text.size());
		size_t pos = 0;

		while (pos < text.size())
		{
			const size_t proto = text.find("://", pos);
			if (proto == std::string::npos)
			{
				out.append(text, pos, text.size() - pos);
				break;
			}

			out.append(text, pos, proto + 3 - pos);
			pos = proto + 3;

			size_t urlEnd = pos;
			size_t atSign = std::string::npos;
			while (urlEnd < text.size() && text[urlEnd] != ' ' && text[urlEnd] != '\n' && text[urlEnd] != '\r' && text[urlEnd] != '\t' &&
				   text[urlEnd] != '\'' && text[urlEnd] != '"')
			{
				if (text[urlEnd] == '@')
					atSign = urlEnd;
				urlEnd++;
			}

			if (atSign != std::string::npos && atSign > pos)
			{
				out.append("***@");
				pos = atSign + 1;
			}
		}

		// Second pass: redact obvious token patterns that may appear
		// outside of URLs (e.g. in error messages).
		auto redactPattern = [](std::string &s, const char *prefix, size_t minTail) {
			const size_t prefLen = std::strlen(prefix);
			size_t p = 0;
			while ((p = s.find(prefix, p)) != std::string::npos)
			{
				size_t end = p + prefLen;
				while (end < s.size() && ((s[end] >= 'a' && s[end] <= 'z') || (s[end] >= 'A' && s[end] <= 'Z') || (s[end] >= '0' && s[end] <= '9') ||
										  s[end] == '_' || s[end] == '-'))
					end++;
				if (end - (p + prefLen) >= minTail)
				{
					s.replace(p + prefLen, end - (p + prefLen), "***");
					end = p + prefLen + 3;
				}
				p = end;
			}
		};

		redactPattern(out, "ghp_", 4);
		redactPattern(out, "gho_", 4);
		redactPattern(out, "ghs_", 4);
		redactPattern(out, "ghu_", 4);
		redactPattern(out, "github_pat_", 4);
		redactPattern(out, "glpat-", 4);

		return out;
	}
}
