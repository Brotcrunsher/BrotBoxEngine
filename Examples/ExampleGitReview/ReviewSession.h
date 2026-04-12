#pragma once

#include "RepoModel.h"
#include "TextDiff.h"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace gitReview
{
	struct ReviewAppState
	{
		static constexpr int kRepoPathCap = 4096;
		static constexpr int kCommitCap = 65536;

		char repoPathUtf8[kRepoPathCap]{};
		RepoSnapshot snapshot;
		std::optional<FileEntry> selection;
		/// Paths highlighted in the file list (Ctrl/Shift). Always includes \c selection->path when a file is selected.
		std::unordered_set<std::string> fileListMultiPaths;
		/// Index into the sorted merged file list for Shift-range selection; \c -1 until first plain or Ctrl click.
		int fileListShiftAnchorIdx = -1;
		ReviewMode reviewMode = ReviewMode::Unstaged;

		std::string leftText;
		/// Row-aligned read-only view of the left side (matches \c InputTextMultiline line layout).
		std::vector<char> leftViewBuffer;
		/// Null-terminated buffer for the right side (ImGui + diff); excludes the trailing '\0' from text comparisons.
		std::vector<char> rightEditBuffer;
		bool rightSideIsWorktreeFile = false;
		bool binaryFile = false;
		std::string loadDiffError;

		char commitMessageUtf8[kCommitCap]{};

		std::string toastText;
		float toastSecondsRemaining = 0.f;

		std::string modalTitle;
		std::string modalBody;
		bool modalOpen = false;

		bool pendingDiscardAsk = false;

		/// When >= 0, the diff scroll view jumps to this Y offset on the next frame (set by the diff map).
		float diffMapScrollTarget = -1.f;
		/// Set to -1 or +1 to request navigation to the previous/next hunk; cleared after processing.
		int diffNavRequest = 0;
		/// After selecting a file, scroll to the first non-equal row once \c hunkStarts is known (not \c diffNavRequest next).
		bool diffScrollToFirstChange = false;
		/// Last known scroll position of the diff view (updated each frame for button enable/disable).
		float diffScrollY = 0.f;

		// Diff-row cache: avoids rebuilding the side-by-side model every frame.
		std::string cachedDiffLeft;
		std::string cachedDiffRight;
		std::vector<DiffRow> cachedDiffRows;
		bool diffCacheLargeFallback = false;
	};

	void showToast(ReviewAppState &app, const std::string &text, float seconds = 4.f);
	void showModal(ReviewAppState &app, const std::string &title, const std::string &body);

	std::string repoRootString(const ReviewAppState &app);

	void tryOpenRepository(ReviewAppState &app, const std::string &path);
	void refreshSnapshot(ReviewAppState &app);
	void reloadDiffForSelection(ReviewAppState &app);

	void setSelection(ReviewAppState &app, FileEntry entry);
	/// Sets the diff focus entry and the file-list multi-selection (paths must exist in the current snapshot list).
	void setFileListPrimaryAndMulti(ReviewAppState &app, FileEntry entry, std::unordered_set<std::string> multiPaths, int shiftAnchorIdx);
	void clearSelection(ReviewAppState &app);

	bool saveWorktreeBuffer(ReviewAppState &app, std::string &err);

	/// Stages the full file from the working tree (\c git add -- …).  For renames, both paths are staged.
	void stageEntry(ReviewAppState &app, const FileEntry &entry, std::string &err);
	/// Unstages the entry.  For renames, both old and new paths are restored.
	void unstageEntry(ReviewAppState &app, const FileEntry &entry, std::string &err);
	/// Discards unstaged worktree changes.  Disabled for renames (sets err).
	void discardWorktreeEntry(ReviewAppState &app, const FileEntry &entry, std::string &err);
	void deleteUntrackedPath(ReviewAppState &app, const std::string &path, std::string &err);

	void commitStaged(ReviewAppState &app, std::string &err);
	void pushUpstream(ReviewAppState &app, std::string &err);

	/// Canonical worktree text derived from the row-aligned editor buffer (refreshes the diff cache).
	std::string rightBufferText(ReviewAppState &app);

	/// Returns a cached view of the side-by-side diff rows, only
	/// recomputing when the underlying texts change.
	const std::vector<DiffRow> &cachedDiffRows(ReviewAppState &app);
}
