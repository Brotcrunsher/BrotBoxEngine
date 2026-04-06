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
		void setRightBufferFromText(std::vector<char> &buf, const std::string &text)
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

		bool writeTempCommitMessage(const std::string &msg, std::filesystem::path &outPath, std::string &err)
		{
			outPath = std::filesystem::temp_directory_path() / "bbe_example_gitreview_commitmsg.txt";
			std::ofstream f(outPath, std::ios::binary | std::ios::trunc);
			if (!f)
			{
				err = "Could not write temporary commit message file.";
				return false;
			}
			f.write(msg.data(), static_cast<std::streamsize>(msg.size()));
			if (!msg.empty() && msg.back() != '\n')
				f.put('\n');
			if (!f)
			{
				err = "Writing commit message failed.";
				return false;
			}
			return true;
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
		}
		clearSelection(app);
		showToast(app, "Refreshed.", 1.8f);
	}

	void reloadDiffForSelection(ReviewAppState &app)
	{
		app.loadDiffError.clear();
		app.leftText.clear();
		app.rightEditBuffer.clear();
		app.rightEditBuffer.push_back('\0');
		app.rightSideIsWorktreeFile = false;
		app.binaryFile = false;
		if (!app.selection.has_value())
			return;

		const std::string root = repoRootString(app);
		if (root.empty())
			return;

		std::string err;
		std::string rightTmp;
		loadDiffPair(root, app.reviewMode, *app.selection, app.leftText, rightTmp, app.rightSideIsWorktreeFile, app.binaryFile, err);
		setRightBufferFromText(app.rightEditBuffer, rightTmp);
		if (!err.empty())
			app.loadDiffError = err;
	}

	void setSelection(ReviewAppState &app, FileEntry entry)
	{
		app.selection = std::move(entry);
		if (app.selection->section == FileListSection::Untracked)
			app.reviewMode = ReviewMode::Unstaged;
		reloadDiffForSelection(app);
	}

	void clearSelection(ReviewAppState &app)
	{
		app.selection.reset();
		app.leftText.clear();
		app.rightEditBuffer.clear();
		app.rightEditBuffer.push_back('\0');
		app.rightSideIsWorktreeFile = false;
		app.binaryFile = false;
		app.loadDiffError.clear();
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
		return writeFileUtf8(p.string(), rightBufferAsString(app.rightEditBuffer), err);
	}

	void stagePath(ReviewAppState &app, const std::string &path, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			err = "No repository path.";
			return;
		}
		GitRunResult r = runGit(root, { "add", "--", path });
		if (r.exitCode != 0)
			err = trimCopy(r.standardOut + r.standardError);
	}

	void unstagePath(ReviewAppState &app, const std::string &path, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			err = "No repository path.";
			return;
		}
		GitRunResult r = runGit(root, { "restore", "--staged", "--", path });
		if (r.exitCode != 0)
			err = trimCopy(r.standardOut + r.standardError);
	}

	void discardWorktreePath(ReviewAppState &app, const std::string &path, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		if (root.empty())
		{
			err = "No repository path.";
			return;
		}
		GitRunResult r = runGit(root, { "restore", "--", path });
		if (r.exitCode != 0)
			err = trimCopy(r.standardOut + r.standardError);
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

	void commitWithMessageFile(ReviewAppState &app, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		const std::string msg = trimCopy(std::string(app.commitMessageUtf8));
		if (msg.empty())
		{
			err = "Commit message is empty.";
			return;
		}

		std::filesystem::path tmp;
		if (!writeTempCommitMessage(msg, tmp, err))
			return;

		GitRunResult r = runGit(root, { "commit", "-F", tmp.string() });
		std::error_code ec;
		std::filesystem::remove(tmp, ec);
		if (r.exitCode != 0)
			err = trimCopy(r.standardOut + r.standardError);
	}

	void pushUpstream(ReviewAppState &app, std::string &err)
	{
		err.clear();
		const std::string root = repoRootString(app);
		GitRunResult r = runGit(root, { "push" });
		if (r.exitCode != 0)
			err = trimCopy(r.standardOut + r.standardError);
	}

	std::string rightBufferText(const ReviewAppState &app)
	{
		return rightBufferAsString(app.rightEditBuffer);
	}
}
