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
		/// Local commits not present on \c @{upstream}; \c -1 if no upstream / error.
		int commitsAheadOfUpstream = -1;
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

	/// True when the index has multiple stages for \p pathInRepo (merge conflict).
	bool pathHasUnmergedIndex(const std::string &repoRoot, const std::string &pathInRepo);

	/// Reads merge stages \c :1: (base), \c :2: (ours), \c :3: (theirs). Missing stage 1 yields empty base.
	/// Returns false if stages 2 and 3 cannot both be read (caller falls back to \c loadDiffPair).
	bool tryLoadMergeIndexVersions(const std::string &repoRoot, const std::string &pathInRepo, std::string &outBase, std::string &outOurs,
		std::string &outTheirs, std::string &outError);

	/// Content-based heuristic: checks for NUL bytes and high density of
	/// non-text control characters in the first 8 KB.
	bool pathLooksBinaryByContent(const std::string &text);

	/// Extension-based heuristic for common binary formats.
	bool pathLooksBinaryByExtension(const std::string &path);

	/// Source / markup extensions we prefer to treat as text when content looks printable.
	bool pathLooksTextByExtension(const std::string &path);

	/// Sets \p outBinary using the same rules as \c loadDiffPair (content + extension heuristics).
	void decideBinaryAfterLoad(const FileEntry &entry, const std::string &left, const std::string &right, bool &outBinary);
}
