#include "GitRunner.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace gitReview
{
	std::string quoteShellSingleQuotedPath(const std::string &path)
	{
		std::string out = "'";
		for (char c : path)
		{
			if (c == '\'')
				out += "'\\''";
			else
				out += c;
		}
		out += '\'';
		return out;
	}

#ifdef _WIN32
	static std::string quoteCmdPath(const std::string &s)
	{
		std::string r = "\"";
		for (char c : s)
		{
			if (c == '"')
				r += "\\\"";
			else
				r += c;
		}
		r += '"';
		return r;
	}

	static std::string buildWindowsGitCommandLine(const std::string &repoRoot, const std::vector<std::string> &args)
	{
		std::ostringstream oss;
		oss << "git -C " << quoteCmdPath(repoRoot);
		for (const auto &a : args)
		{
			oss << ' ' << quoteCmdPath(a);
		}
		return oss.str();
	}
#endif

	GitRunResult runGit(const std::string &repoRoot, const std::vector<std::string> &args)
	{
		GitRunResult result;
#ifdef _WIN32
		// _popen runs through cmd.exe; merge stderr into stdout for a single UX stream.
		std::string full = buildWindowsGitCommandLine(repoRoot, args) + " 2>&1";
		FILE *pipe = _popen(full.c_str(), "rb");
		if (!pipe)
		{
			result.standardError = "Failed to start git (Windows popen).";
			result.exitCode = -1;
			return result;
		}
		std::string combined;
		char buf[4096];
		while (true)
		{
			const size_t n = fread(buf, 1, sizeof(buf), pipe);
			if (n == 0)
				break;
			combined.append(buf, n);
		}
		const int st = _pclose(pipe);
		result.standardOut = std::move(combined);
		result.exitCode = (st >= 0) ? st : -1;
		return result;
#else
		std::array<int, 2> outPipe{};
		std::array<int, 2> errPipe{};
		if (pipe(outPipe.data()) != 0 || pipe(errPipe.data()) != 0)
		{
			result.standardError = "pipe() failed.";
			return result;
		}

		const pid_t pid = fork();
		if (pid < 0)
		{
			close(outPipe[0]);
			close(outPipe[1]);
			close(errPipe[0]);
			close(errPipe[1]);
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

		close(outPipe[1]);
		close(errPipe[1]);

		auto drainFd = [](int fd, std::string &into) {
			char buf[4096];
			for (;;)
			{
				const ssize_t n = read(fd, buf, sizeof(buf));
				if (n < 0)
					break;
				if (n == 0)
					break;
				into.append(buf, static_cast<size_t>(n));
			}
			close(fd);
		};

		drainFd(outPipe[0], result.standardOut);
		drainFd(errPipe[0], result.standardError);

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
#endif
	}
}
