#pragma once

#include "RepoModel.h"

#include <optional>
#include <string>
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
		ReviewMode reviewMode = ReviewMode::Unstaged;

		std::string leftText;
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
		bool pendingUntrackedDeleteAsk = false;
	};

	void showToast(ReviewAppState &app, const std::string &text, float seconds = 4.f);
	void showModal(ReviewAppState &app, const std::string &title, const std::string &body);

	std::string repoRootString(const ReviewAppState &app);

	void tryOpenRepository(ReviewAppState &app, const std::string &path);
	void refreshSnapshot(ReviewAppState &app);
	void reloadDiffForSelection(ReviewAppState &app);

	void setSelection(ReviewAppState &app, FileEntry entry);
	void clearSelection(ReviewAppState &app);

	bool saveWorktreeBuffer(ReviewAppState &app, std::string &err);
	void stagePath(ReviewAppState &app, const std::string &path, std::string &err);
	void unstagePath(ReviewAppState &app, const std::string &path, std::string &err);
	void discardWorktreePath(ReviewAppState &app, const std::string &path, std::string &err);
	void deleteUntrackedPath(ReviewAppState &app, const std::string &path, std::string &err);
	void commitWithMessageFile(ReviewAppState &app, std::string &err);
	void pushUpstream(ReviewAppState &app, std::string &err);

	/// Text of \c rightEditBuffer without the trailing null terminator (for diffing and saving).
	std::string rightBufferText(const ReviewAppState &app);
}
