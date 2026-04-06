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

	/// Runs `git` with the given arguments after `-C repoRoot`.
	/// Uses a child process; does not change the process cwd.
	GitRunResult runGit(const std::string &repoRoot, const std::vector<std::string> &args);

	/// Like runGit, but feeds \p stdinData to the child's stdin and closes it.
	GitRunResult runGitWithStdin(const std::string &repoRoot, const std::vector<std::string> &args, const std::string &stdinData);

	/// Redacts credentialed URLs and obvious token patterns from Git
	/// output before it is shown in the UI.
	std::string redactGitOutput(const std::string &text);
}
