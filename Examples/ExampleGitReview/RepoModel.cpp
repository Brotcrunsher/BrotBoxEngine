#include "RepoModel.h"
#include "GitRunner.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace gitReview
{
	namespace
	{
		std::string trim(std::string s)
		{
			while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
				s.erase(s.begin());
			while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
				s.pop_back();
			return s;
		}

		std::vector<std::string> splitLines(const std::string &s)
		{
			std::vector<std::string> lines;
			std::istringstream iss(s);
			std::string line;
			while (std::getline(iss, line))
			{
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				lines.push_back(std::move(line));
			}
			return lines;
		}

		std::vector<std::string> splitTab(const std::string &line, size_t minParts)
		{
			std::vector<std::string> parts;
			size_t pos = 0;
			while (pos < line.size())
			{
				const size_t tab = line.find('\t', pos);
				if (tab == std::string::npos)
				{
					parts.push_back(line.substr(pos));
					break;
				}
				parts.push_back(line.substr(pos, tab - pos));
				pos = tab + 1;
			}
			while (parts.size() < minParts)
				parts.emplace_back();
			return parts;
		}

		void appendNameStatus(std::unordered_map<std::string, FileEntry> &byPath, const std::vector<std::string> &lines, FileListSection section)
		{
			for (const std::string &line : lines)
			{
				if (line.empty())
					continue;
				const auto parts = splitTab(line, 3);
				if (parts[0].empty())
					continue;
				const std::string &status = parts[0];
				FileEntry fe;
				fe.section = section;
				if (status[0] == 'R' || status[0] == 'C')
				{
					fe.kind = ChangeKind::Renamed;
					if (parts.size() < 3 || parts[1].empty() || parts[2].empty())
						continue;
					fe.renameFrom = parts[1];
					fe.path = parts[2];
				}
				else if (status[0] == 'A')
				{
					fe.kind = ChangeKind::Added;
					if (parts.size() < 2)
						continue;
					fe.path = parts[1];
				}
				else if (status[0] == 'D')
				{
					fe.kind = ChangeKind::Deleted;
					if (parts.size() < 2)
						continue;
					fe.path = parts[1];
				}
				else
				{
					fe.kind = ChangeKind::Modified;
					if (parts.size() < 2)
						continue;
					fe.path = parts[1];
				}
				byPath[fe.path] = std::move(fe);
			}
		}
	}

	bool pathLooksBinaryByContent(const std::string &text)
	{
		// NUL bytes are a strong signal; keep heuristic small for V1.
		return text.find('\0') != std::string::npos;
	}

	bool isGitRepositoryRoot(const std::string &path, std::string &outError)
	{
		outError.clear();
		std::error_code ec;
		if (!std::filesystem::exists(std::filesystem::path(path), ec))
		{
			outError = "Path does not exist.";
			return false;
		}
		GitRunResult r = runGit(path, { "rev-parse", "--is-inside-work-tree" });
		if (r.exitCode != 0)
		{
			outError = "Not a Git repository (rev-parse failed).\n" + trim(r.standardOut + r.standardError);
			return false;
		}
		const std::string v = trim(r.standardOut);
		if (v != "true")
		{
			outError = "Not a Git work tree.";
			return false;
		}
		return true;
	}

	void refreshRepository(const std::string &repoRoot, RepoSnapshot &outSnap, std::string &outError)
	{
		outError.clear();
		outSnap = RepoSnapshot();
		outSnap.root = repoRoot;

		GitRunResult branch = runGit(repoRoot, { "rev-parse", "--abbrev-ref", "HEAD" });
		if (branch.exitCode == 0)
			outSnap.branchHint = trim(branch.standardOut);

		GitRunResult head = runGit(repoRoot, { "rev-parse", "--short", "HEAD" });
		if (head.exitCode == 0)
			outSnap.headShort = trim(head.standardOut);

		GitRunResult unst = runGit(repoRoot, { "diff", "--find-renames", "--name-status" });
		GitRunResult stgd = runGit(repoRoot, { "diff", "--cached", "--find-renames", "--name-status" });
		GitRunResult untr = runGit(repoRoot, { "ls-files", "-o", "--exclude-standard" });

		if (unst.exitCode != 0)
		{
			outError = "git diff failed.\n" + trim(unst.standardOut + unst.standardError);
			return;
		}
		if (stgd.exitCode != 0)
		{
			outError = "git diff --cached failed.\n" + trim(stgd.standardOut + stgd.standardError);
			return;
		}
		if (untr.exitCode != 0)
		{
			outError = "git ls-files failed.\n" + trim(untr.standardOut + untr.standardError);
			return;
		}

		std::unordered_map<std::string, FileEntry> staged;
		std::unordered_map<std::string, FileEntry> unstaged;
		appendNameStatus(staged, splitLines(stgd.standardOut), FileListSection::Staged);
		appendNameStatus(unstaged, splitLines(unst.standardOut), FileListSection::Unstaged);

		std::unordered_set<std::string> untrackedPaths;
		for (const auto &line : splitLines(untr.standardOut))
		{
			const std::string p = trim(line);
			if (!p.empty())
				untrackedPaths.insert(p);
		}

		// Untracked section: only paths not present as tracked entries in staged/unstaged maps.
		for (const auto &p : untrackedPaths)
		{
			if (staged.count(p) || unstaged.count(p))
				continue;
			FileEntry fe;
			fe.section = FileListSection::Untracked;
			fe.kind = ChangeKind::Added;
			fe.path = p;
			outSnap.entries.push_back(std::move(fe));
		}

		// Unstaged entries (skip if only appears because identical to staged-only — diff already excludes clean files).
		for (auto &kv : unstaged)
			outSnap.entries.push_back(std::move(kv.second));

		// Staged entries
		for (auto &kv : staged)
			outSnap.entries.push_back(std::move(kv.second));

		std::sort(outSnap.entries.begin(), outSnap.entries.end(), [](const FileEntry &a, const FileEntry &b) {
			if (a.section != b.section)
				return static_cast<int>(a.section) < static_cast<int>(b.section);
			return a.path < b.path;
		});
	}

	std::string readFileUtf8(const std::string &absolutePath, std::string &outError)
	{
		outError.clear();
		std::ifstream f(absolutePath, std::ios::binary);
		if (!f)
		{
			outError = "Could not open file for reading.";
			return {};
		}
		std::ostringstream oss;
		oss << f.rdbuf();
		return oss.str();
	}

	bool writeFileUtf8(const std::string &absolutePath, const std::string &content, std::string &outError)
	{
		outError.clear();
		std::ofstream f(absolutePath, std::ios::binary | std::ios::trunc);
		if (!f)
		{
			outError = "Could not open file for writing.";
			return false;
		}
		f.write(content.data(), static_cast<std::streamsize>(content.size()));
		if (!f)
		{
			outError = "Writing file failed.";
			return false;
		}
		return true;
	}

	void loadDiffPair(const std::string &repoRoot, ReviewMode mode, const FileEntry &entry, std::string &outLeft, std::string &outRight,
		bool &outRightIsWorktreeFile, bool &outBinary, std::string &outError)
	{
		outLeft.clear();
		outRight.clear();
		outRightIsWorktreeFile = false;
		outBinary = false;
		outError.clear();

		const std::filesystem::path root(repoRoot);
		const std::filesystem::path workPath = root / std::filesystem::path(entry.path);

		auto showHeadPath = [&](const std::string &pathInRepo, std::string &into) {
			GitRunResult r = runGit(repoRoot, { "show", std::string("HEAD:") + pathInRepo });
			if (r.exitCode != 0)
			{
				into.clear();
				return false;
			}
			into = r.standardOut;
			return true;
		};
		auto showIndexPath = [&](const std::string &pathInRepo, std::string &into) {
			GitRunResult r = runGit(repoRoot, { "show", std::string(":") + pathInRepo });
			if (r.exitCode != 0)
			{
				into.clear();
				return false;
			}
			into = r.standardOut;
			return true;
		};

		if (entry.section == FileListSection::Untracked)
		{
			outLeft.clear();
			outRight = readFileUtf8(workPath.string(), outError);
			outRightIsWorktreeFile = true;
			if (!outError.empty())
				return;
			outBinary = pathLooksBinaryByContent(outRight);
			return;
		}

		if (mode == ReviewMode::Unstaged)
		{
			if (entry.kind == ChangeKind::Deleted)
			{
				if (!showIndexPath(entry.path, outLeft))
				{
					outError = "Could not read index version of deleted path.";
					return;
				}
				outRight.clear();
				outRightIsWorktreeFile = false;
				outBinary = pathLooksBinaryByContent(outLeft);
				return;
			}

			if (entry.kind == ChangeKind::Renamed)
			{
				// Unstaged rename: index still has old name; worktree has new name.
				if (!showIndexPath(entry.renameFrom, outLeft))
				{
					outError = "Could not read old path at index for rename.";
					return;
				}
				outRight = readFileUtf8(workPath.string(), outError);
				outRightIsWorktreeFile = true;
				if (!outError.empty())
					return;
				outBinary = pathLooksBinaryByContent(outLeft) || pathLooksBinaryByContent(outRight);
				return;
			}

			// Modified or added (unstaged means index vs worktree; added file may exist in index as empty?)
			if (entry.kind == ChangeKind::Added)
			{
				// New in index not typical for unstaged-only; still try index then worktree.
				(void)showIndexPath(entry.path, outLeft);
			}
			else
			{
				if (!showIndexPath(entry.path, outLeft))
				{
					outError = "Could not read staged/index version.";
					return;
				}
			}
			outRight = readFileUtf8(workPath.string(), outError);
			outRightIsWorktreeFile = true;
			if (!outError.empty())
				return;
			outBinary = pathLooksBinaryByContent(outLeft) || pathLooksBinaryByContent(outRight);
			return;
		}

		// Staged: HEAD vs index
		if (entry.kind == ChangeKind::Deleted)
		{
			if (!showHeadPath(entry.path, outLeft))
			{
				outError = "Could not read HEAD version of deleted path.";
				return;
			}
			outRight.clear();
			outRightIsWorktreeFile = false;
			outBinary = pathLooksBinaryByContent(outLeft);
			return;
		}

		if (entry.kind == ChangeKind::Renamed)
		{
			if (!showHeadPath(entry.renameFrom, outLeft))
			{
				outError = "Could not read HEAD old path for rename.";
				return;
			}
			if (!showIndexPath(entry.path, outRight))
			{
				outError = "Could not read index new path for rename.";
				return;
			}
			outRightIsWorktreeFile = false;
			outBinary = pathLooksBinaryByContent(outLeft) || pathLooksBinaryByContent(outRight);
			return;
		}

		if (entry.kind == ChangeKind::Added)
		{
			outLeft.clear();
			if (!showIndexPath(entry.path, outRight))
			{
				outError = "Could not read new file at index.";
				return;
			}
			outRightIsWorktreeFile = false;
			outBinary = pathLooksBinaryByContent(outRight);
			return;
		}

		// Modified
		if (!showHeadPath(entry.path, outLeft))
		{
			outError = "Could not read HEAD version.";
			return;
		}
		if (!showIndexPath(entry.path, outRight))
		{
			outError = "Could not read index version.";
			return;
		}
		outRightIsWorktreeFile = false;
		outBinary = pathLooksBinaryByContent(outLeft) || pathLooksBinaryByContent(outRight);
	}
}
