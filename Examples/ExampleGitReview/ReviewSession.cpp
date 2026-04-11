#include "ReviewSession.h"
#include "GitRunner.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>

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

		if (!selPath.empty())
		{
			for (const auto &e : app.snapshot.entries)
			{
				if (e.path == selPath && e.section == selSection && e.kind == selKind && e.renameFrom == selRename)
				{
					app.selection = e;
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
	}

	void setSelection(ReviewAppState &app, FileEntry entry)
	{
		app.selection = std::move(entry);
		if (app.selection->section == FileListSection::Untracked)
			app.reviewMode = ReviewMode::Unstaged;
		reloadDiffForSelection(app);
		app.diffScrollY = 0.f;
		app.diffNavRequest = 1;
	}

	void clearSelection(ReviewAppState &app)
	{
		app.selection.reset();
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
	}

	bool saveWorktreeBuffer(ReviewAppState &app, std::string &err)
	{
		err.clear();
		if (!app.selection.has_value() || !app.rightSideIsWorktreeFile)
		{
			err = "Nothing to save (editable side is not the working tree file).";
			return false;
		}
		const std::filesystem::path p = std::filesystem::path(repoRootString(app)) / std::filesystem::path(app.selection->path);
		return writeFileUtf8(p.string(), rightBufferText(app), err);
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
		cachedDiffRows(app);
		return app.cachedDiffRight;
	}
}
