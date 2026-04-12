#include "ReviewSession.h"
#include "GitRunner.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <vector>

namespace gitReview
{
	namespace
	{
		void setBufferFromText(std::vector<char> &buf, const std::string &text)
		{
			buf.assign(text.begin(), text.end());
			buf.push_back('\0');
		}

		std::string rightBufferAsString(const std::vector<char> &buf)
		{
			if (buf.empty())
				return {};
			if (buf.back() == '\0')
			{
				if (buf.size() <= 1)
					return {};
				return std::string(buf.begin(), buf.end() - 1);
			}
			return std::string(buf.begin(), buf.end());
		}

		std::string trimCopy(std::string s)
		{
			auto notSpace = [](unsigned char c) { return !std::isspace(c); };
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
			s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
			return s;
		}

		bool hasNonWhitespace(const char *s)
		{
			for (; *s; ++s)
			{
				if (!std::isspace(static_cast<unsigned char>(*s)))
					return true;
			}
			return false;
		}

		/// Sanitize git output for user-facing display.
		std::string sanitizedGitError(const GitRunResult &r)
		{
			std::string combined = r.standardOut;
			if (!r.standardError.empty())
			{
				if (!combined.empty())
					combined += '\n';
				combined += r.standardError;
			}
			return redactGitOutput(trimCopy(std::move(combined)));
		}
	}

	namespace detail
	{
		struct MergedFile
		{
			std::string path;
			bool hasStaged = false;
			bool hasUnstaged = false;
			bool isUntracked = false;
			FileEntry stagedEntry;
			FileEntry unstagedEntry;
		};

		std::vector<MergedFile> mergeSnapshotEntries(const RepoSnapshot &snapshot)
		{
			std::vector<MergedFile> merged;
			for (const FileEntry &e : snapshot.entries)
			{
				auto it = std::find_if(merged.begin(), merged.end(),
					[&](const MergedFile &m) { return m.path == e.path; });
				if (it == merged.end())
				{
					MergedFile mf;
					mf.path = e.path;
					if (e.section == FileListSection::Staged)
					{
						mf.hasStaged = true;
						mf.stagedEntry = e;
					}
					else
					{
						mf.hasUnstaged = true;
						mf.isUntracked = (e.section == FileListSection::Untracked);
						mf.unstagedEntry = e;
					}
					merged.push_back(std::move(mf));
				}
				else
				{
					if (e.section == FileListSection::Staged)
					{
						it->hasStaged = true;
						it->stagedEntry = e;
					}
					else
					{
						it->hasUnstaged = true;
						it->isUntracked = (e.section == FileListSection::Untracked);
						it->unstagedEntry = e;
					}
				}
			}
			std::sort(merged.begin(), merged.end(),
				[](const MergedFile &a, const MergedFile &b) { return a.path < b.path; });
			return merged;
		}

	}

	void showToast(ReviewAppState &app, const std::string &text, float seconds)
	{
		app.toastText = text;
		app.toastSecondsRemaining = seconds;
	}

	void showModal(ReviewAppState &app, const std::string &title, const std::string &body)
	{
		app.modalTitle = title;
		app.modalBody = body;
		app.modalOpen = true;
	}

	std::string repoRootString(const ReviewAppState &app)
	{
		return trimCopy(std::string(app.repoPathUtf8));
	}

	void tryOpenRepository(ReviewAppState &app, const std::string &path)
	{
		std::string err;
		const std::string p = trimCopy(path);
		if (p.size() >= ReviewAppState::kRepoPathCap)
		{
			showModal(app, "Cannot open repository", "Path is too long for the path buffer.");
			return;
		}
		std::memcpy(app.repoPathUtf8, p.c_str(), p.size() + 1);

		if (!isGitRepositoryRoot(p, err))
		{
			showModal(app, "Not a Git repository", err.empty() ? "Unknown error." : err);
			app.snapshot = RepoSnapshot();
			clearSelection(app);
			return;
		}

		refreshSnapshot(app);
		showToast(app, "Repository opened.", 2.5f);
	}

	void refreshSnapshot(ReviewAppState &app)
	{
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			showModal(app, "Refresh", "No repository path set.");
			return;
		}
		std::string err;
		if (!isGitRepositoryRoot(root, err))
		{
			showModal(app, "Not a Git repository", err);
			return;
		}

		RepoSnapshot next;
		refreshRepository(root, next, err);
		if (!err.empty())
		{
			showModal(app, "Refresh failed", err);
			return;
		}

		const std::string selPath = app.selection ? app.selection->path : std::string();
		const auto selSection = app.selection ? app.selection->section : FileListSection::Unstaged;
		const auto selKind = app.selection ? app.selection->kind : ChangeKind::Modified;
		const std::string selRename = app.selection ? app.selection->renameFrom : std::string();

		app.snapshot = std::move(next);

		{
			const std::vector<detail::MergedFile> mergedFresh = detail::mergeSnapshotEntries(app.snapshot);
			for (auto it = app.fileListMultiPaths.begin(); it != app.fileListMultiPaths.end();)
			{
				const bool keep = std::any_of(mergedFresh.begin(), mergedFresh.end(),
					[&](const detail::MergedFile &m) { return m.path == *it; });
				if (!keep)
					it = app.fileListMultiPaths.erase(it);
				else
					++it;
			}
			if (app.fileListShiftAnchorIdx < 0 || app.fileListShiftAnchorIdx >= static_cast<int>(mergedFresh.size()))
				app.fileListShiftAnchorIdx = -1;
		}

		if (!selPath.empty())
		{
			for (const auto &e : app.snapshot.entries)
			{
				if (e.path == selPath && e.section == selSection && e.kind == selKind && e.renameFrom == selRename)
				{
					app.selection = e;
					if (app.fileListMultiPaths.empty())
						app.fileListMultiPaths.insert(selPath);
					reloadDiffForSelection(app);
					showToast(app, "Refreshed.", 1.8f);
					return;
				}
			}
		// Fallback: match by path only (section may change after stage/unstage).
		for (const auto &e : app.snapshot.entries)
		{
			if (e.path == selPath)
			{
				app.selection = e;
				if (e.section == FileListSection::Staged)
					app.reviewMode = ReviewMode::Staged;
				else
					app.reviewMode = ReviewMode::Unstaged;
				if (app.fileListMultiPaths.empty())
					app.fileListMultiPaths.insert(selPath);
				reloadDiffForSelection(app);
				showToast(app, "Refreshed.", 1.8f);
				return;
			}
		}
		}
		clearSelection(app);
		showToast(app, "Refreshed.", 1.8f);
	}

	void reloadDiffForSelection(ReviewAppState &app)
	{
		app.loadDiffError.clear();
		app.rightWorktreeSavedCanon.clear();
		app.leftText.clear();
		app.leftViewBuffer.clear();
		app.leftViewBuffer.push_back('\0');
		app.rightEditBuffer.clear();
		app.rightEditBuffer.push_back('\0');
		app.rightSideIsWorktreeFile = false;
		app.binaryFile = false;
		app.cachedDiffLeft.clear();
		app.cachedDiffRight.clear();
		app.cachedDiffRows.clear();
		app.diffCacheLargeFallback = false;
		if (!app.selection.has_value())
			return;

		const std::string root = repoRootString(app);
		if (root.empty())
			return;

		std::string err;
		std::string rightTmp;
		loadDiffPair(root, app.reviewMode, *app.selection, app.leftText, rightTmp, app.rightSideIsWorktreeFile, app.binaryFile, err);
		setBufferFromText(app.rightEditBuffer, rightTmp);
		if (!err.empty())
			app.loadDiffError = err;
		if (err.empty() && app.rightSideIsWorktreeFile && !app.binaryFile)
			app.rightWorktreeSavedCanon = rightBufferText(app);
	}

	bool rightWorktreeBufferHasUnsavedEdits(ReviewAppState &app)
	{
		if (!app.selection.has_value() || !app.rightSideIsWorktreeFile || app.binaryFile || !app.loadDiffError.empty())
			return false;
		return rightBufferText(app) != app.rightWorktreeSavedCanon;
	}

	void setFileListPrimaryAndMulti(ReviewAppState &app, FileEntry entry, std::unordered_set<std::string> multiPaths, int shiftAnchorIdx)
	{
		app.selection = std::move(entry);
		app.fileListMultiPaths = std::move(multiPaths);
		app.fileListShiftAnchorIdx = shiftAnchorIdx;
		if (app.selection->section == FileListSection::Untracked)
			app.reviewMode = ReviewMode::Unstaged;
		reloadDiffForSelection(app);
		app.diffScrollY = 0.f;
		app.diffNavRequest = 0;
		app.diffScrollToFirstChange = true;
	}

	void setSelection(ReviewAppState &app, FileEntry entry)
	{
		std::unordered_set<std::string> one;
		one.insert(entry.path);
		setFileListPrimaryAndMulti(app, std::move(entry), std::move(one), -1);
	}

	void clearSelection(ReviewAppState &app)
	{
		app.selection.reset();
		app.fileListMultiPaths.clear();
		app.fileListShiftAnchorIdx = -1;
		app.pendingGitignoreAsk = false;
		app.diffScrollToFirstChange = false;
		app.leftText.clear();
		app.leftViewBuffer.clear();
		app.leftViewBuffer.push_back('\0');
		app.rightEditBuffer.clear();
		app.rightEditBuffer.push_back('\0');
		app.rightSideIsWorktreeFile = false;
		app.binaryFile = false;
		app.loadDiffError.clear();
		app.cachedDiffLeft.clear();
		app.cachedDiffRight.clear();
		app.cachedDiffRows.clear();
		app.diffCacheLargeFallback = false;
		app.rightWorktreeSavedCanon.clear();
	}

	bool saveWorktreeBuffer(ReviewAppState &app, std::string &err)
	{
		err.clear();
		if (!app.selection.has_value() || !app.rightSideIsWorktreeFile)
		{
			err = "Nothing to save (editable side is not the working tree file).";
			return false;
		}
		if (app.binaryFile)
		{
			err = "Binary files cannot be edited in this view; save is disabled.";
			return false;
		}
		const std::filesystem::path p = std::filesystem::path(repoRootString(app)) / std::filesystem::path(app.selection->path);
		const std::string toWrite = rightBufferText(app);
		if (!writeFileUtf8(p.string(), toWrite, err))
			return false;
		app.rightWorktreeSavedCanon = toWrite;
		return true;
	}

	void stageEntry(ReviewAppState &app, const FileEntry &entry, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			err = "No repository path.";
			return;
		}

		if (entry.kind == ChangeKind::Renamed && !entry.renameFrom.empty())
		{
			GitRunResult r = runGit(root, { "add", "--", entry.renameFrom, entry.path });
			if (r.exitCode != 0)
				err = sanitizedGitError(r);
		}
		else
		{
			GitRunResult r = runGit(root, { "add", "--", entry.path });
			if (r.exitCode != 0)
				err = sanitizedGitError(r);
		}
	}

	void unstageEntry(ReviewAppState &app, const FileEntry &entry, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			err = "No repository path.";
			return;
		}

		if (entry.kind == ChangeKind::Renamed && !entry.renameFrom.empty())
		{
			GitRunResult r = runGit(root, { "restore", "--staged", "--", entry.renameFrom, entry.path });
			if (r.exitCode != 0)
				err = sanitizedGitError(r);
		}
		else
		{
			GitRunResult r = runGit(root, { "restore", "--staged", "--", entry.path });
			if (r.exitCode != 0)
				err = sanitizedGitError(r);
		}
	}

	void discardWorktreeEntry(ReviewAppState &app, const FileEntry &entry, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			err = "No repository path.";
			return;
		}

		if (entry.kind == ChangeKind::Renamed)
		{
			err = "Discarding a rename is not supported yet.  Unstage the rename first, then discard the individual paths.";
			return;
		}

		GitRunResult r = runGit(root, { "restore", "--", entry.path });
		if (r.exitCode != 0)
			err = sanitizedGitError(r);
	}

	void deleteUntrackedPath(ReviewAppState &app, const std::string &path, std::string &err)
	{
		err.clear();
		const std::filesystem::path p = std::filesystem::path(repoRootString(app)) / std::filesystem::path(path);
		std::error_code ec;
		if (!std::filesystem::exists(p, ec))
		{
			err = "File does not exist.";
			return;
		}
		if (!std::filesystem::remove(p, ec))
			err = ec.message();
	}

	namespace
	{
		static std::string trimGitignoreLine(std::string line)
		{
			while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
				line.pop_back();
			return line;
		}

		static bool isSafeGitignorePath(const std::string &p)
		{
			if (p.empty())
				return false;
			if (p.front() == '/')
				return false;
			if (p.find("..") != std::string::npos)
				return false;
			return true;
		}

		static void collectGitignoreLines(const std::string &content, std::unordered_set<std::string> &outTrimmedLines)
		{
			size_t i = 0;
			while (i < content.size())
			{
				const size_t j = content.find('\n', i);
				const std::string raw = (j == std::string::npos) ? content.substr(i) : content.substr(i, j - i);
				const std::string t = trimGitignoreLine(raw);
				if (!t.empty())
					outTrimmedLines.insert(t);
				if (j == std::string::npos)
					break;
				i = j + 1;
			}
		}
	} // namespace

	int appendPathsToGitignore(const std::string &repoRoot, const std::vector<std::string> &pathsRel, std::string &err)
	{
		err.clear();
		if (repoRoot.empty())
		{
			err = "No repository path.";
			return -1;
		}
		if (pathsRel.empty())
			return 0;

		const std::filesystem::path giFs = std::filesystem::path(repoRoot) / ".gitignore";
		const std::string giPath = giFs.string();

		std::string content;
		if (std::filesystem::exists(giFs))
		{
			content = readFileUtf8(giPath, err);
			if (!err.empty())
				return -1;
		}

		std::unordered_set<std::string> existing;
		collectGitignoreLines(content, existing);

		std::string toAppend;
		int added = 0;
		for (const std::string &p : pathsRel)
		{
			if (!isSafeGitignorePath(p))
			{
				err = "Refusing unsafe path for .gitignore: " + p;
				return -1;
			}
			if (existing.count(p))
				continue;
			toAppend += p;
			toAppend.push_back('\n');
			existing.insert(p);
			++added;
		}
		if (added == 0)
			return 0;

		if (!content.empty() && content.back() != '\n')
			content.push_back('\n');
		content += toAppend;

		if (!writeFileUtf8(giPath, content, err))
			return -1;
		return added;
	}

	void commitStaged(ReviewAppState &app, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		const std::string msg(app.commitMessageUtf8);
		if (!hasNonWhitespace(app.commitMessageUtf8))
		{
			err = "Commit message is empty.";
			return;
		}
		const std::vector<detail::MergedFile> merged = detail::mergeSnapshotEntries(app.snapshot);
		std::vector<std::string> pathsForCommit;
		pathsForCommit.reserve(merged.size());
		for (const detail::MergedFile &mf : merged)
		{
			if (mf.hasStaged && !mf.hasUnstaged)
				pathsForCommit.push_back(mf.path);
		}
		if (pathsForCommit.empty())
		{
			err = "Nothing to commit: no files are fully staged (no checkboxes ticked).";
			return;
		}

		{
			GitRunResult r = runGit(root, { "restore", "--staged", ":/" });
			if (r.exitCode != 0)
			{
				err = sanitizedGitError(r);
				return;
			}
		}

		for (const std::string &path : pathsForCommit)
		{
			const detail::MergedFile *mf = nullptr;
			for (const detail::MergedFile &m : merged)
			{
				if (m.path == path)
				{
					mf = &m;
					break;
				}
			}
			if (!mf)
			{
				err = "Internal error: checked path \"" + path + "\" is missing from the file list. Try refreshing.";
				return;
			}
			const FileEntry &toStageEntry = mf->hasUnstaged ? mf->unstagedEntry : mf->stagedEntry;
			stageEntry(app, toStageEntry, err);
			if (!err.empty())
				return;
		}

		GitRunResult r = runGitWithStdin(root, { "commit", "-F", "-" }, msg);
		if (r.exitCode != 0)
			err = sanitizedGitError(r);
	}

	void pushUpstream(ReviewAppState &app, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		GitRunResult r = runGit(root, { "push" });
		if (r.exitCode != 0)
			err = sanitizedGitError(r);
	}

	const std::vector<DiffRow> &cachedDiffRows(ReviewAppState &app)
	{
		if (app.binaryFile)
			return app.cachedDiffRows;
		const std::string aligned = rightBufferAsString(app.rightEditBuffer);
		const std::vector<std::string> pl = splitLinesForDiff(aligned);

		std::string canon;
		if (!app.cachedDiffRows.empty() && pl.size() == app.cachedDiffRows.size())
			canon = canonicalFromAlignedRightBuffer(pl, app.cachedDiffRows);
		else
			canon = joinLinesForDiff(pl);

		if (app.leftText != app.cachedDiffLeft || canon != app.cachedDiffRight)
		{
			app.cachedDiffLeft = app.leftText;
			app.cachedDiffRight = canon;
			app.cachedDiffRows = buildSideBySideRows(app.leftText, canon, app.diffCacheLargeFallback);
			const std::string alignedFix = buildAlignedRightBuffer(canon, app.cachedDiffRows);
			if (alignedFix != aligned)
				setBufferFromText(app.rightEditBuffer, alignedFix);
			setBufferFromText(app.leftViewBuffer, buildAlignedLeftBuffer(app.cachedDiffRows));
		}
		return app.cachedDiffRows;
	}

	std::string rightBufferText(ReviewAppState &app)
	{
		if (app.binaryFile)
			return rightBufferAsString(app.rightEditBuffer);
		cachedDiffRows(app);
		return app.cachedDiffRight;
	}
}
