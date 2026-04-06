#pragma once

#include <string>
#include <vector>

namespace gitReview
{
	struct GitRunResult
	{
		int exitCode = -1;
		std::string standardOut;
		std::string standardError;
	};

	/// Runs `git` with the given arguments after `-C repoRoot`. Uses a child process; does not change the process cwd.
	GitRunResult runGit(const std::string &repoRoot, const std::vector<std::string> &args);

	std::string quoteShellSingleQuotedPath(const std::string &path);
}
