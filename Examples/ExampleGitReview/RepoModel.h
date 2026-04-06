#pragma once

#include <string>
#include <vector>

namespace gitReview
{
	enum class ReviewMode
	{
		/// Worktree vs index.
		Unstaged,
		/// Index vs HEAD.
		Staged,
	};

	enum class FileListSection
	{
		Untracked,
		Unstaged,
		Staged,
	};

	enum class ChangeKind
	{
		Modified,
		Added,
		Deleted,
		Renamed,
	};

	struct FileEntry
	{
		FileListSection section = FileListSection::Unstaged;
		ChangeKind kind = ChangeKind::Modified;
		/// Path as used by git (POSIX slashes are fine on all platforms via git).
		std::string path;
		/// Set when \c kind == Renamed (source path).
		std::string renameFrom;
	};

	struct RepoSnapshot
	{
		std::string root;
		std::string headShort;
		std::string branchHint;
		std::vector<FileEntry> entries;
	};

	/// Validates \p path as a git work tree root (must contain `.git` or be a gitdir).
	bool isGitRepositoryRoot(const std::string &path, std::string &outError);

	/// Fills snapshot; on failure sets \p outError and leaves snapshot partially empty.
	void refreshRepository(const std::string &repoRoot, RepoSnapshot &outSnap, std::string &outError);

	std::string readFileUtf8(const std::string &absolutePath, std::string &outError);

	bool writeFileUtf8(const std::string &absolutePath, const std::string &content, std::string &outError);

	/// Left/right text for diff UI. \p outLeft/\p outRight may be empty for absent sides.
	void loadDiffPair(const std::string &repoRoot, ReviewMode mode, const FileEntry &entry, std::string &outLeft, std::string &outRight,
		bool &outRightIsWorktreeFile, bool &outBinary, std::string &outError);

	bool pathLooksBinaryByContent(const std::string &text);
}
