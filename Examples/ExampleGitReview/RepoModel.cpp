#include "RepoModel.h"
#include "GitRunner.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <climits>
#include <cstdlib>
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

		// ── NUL-delimited parsers ────────────────────────────────────

		/// Reads a NUL-terminated field from \p raw starting at \p pos.
		/// Advances pos past the NUL.  Returns false if no NUL was found.
		bool readNulField(const std::string &raw, size_t &pos, std::string &out)
		{
			const size_t nul = raw.find('\0', pos);
			if (nul == std::string::npos)
			{
				if (pos < raw.size())
				{
					out = raw.substr(pos);
					pos = raw.size();
					return true;
				}
				return false;
			}
			out = raw.substr(pos, nul - pos);
			pos = nul + 1;
			return true;
		}

		/// Parses `git diff -z --name-status` output.
		/// Format: STATUS\0path[\0path2]\0  (renames/copies have two paths).
		void appendNameStatusNul(std::unordered_map<std::string, FileEntry> &byPath, const std::string &raw, FileListSection section)
		{
			size_t pos = 0;
			while (pos < raw.size())
			{
				std::string status;
				if (!readNulField(raw, pos, status) || status.empty())
					continue;

				const char sc = status[0];
				FileEntry fe;
				fe.section = section;

				if (sc == 'R' || sc == 'C')
				{
					fe.kind = ChangeKind::Renamed;
					if (!readNulField(raw, pos, fe.renameFrom))
						break;
					if (!readNulField(raw, pos, fe.path))
						break;
				}
				else
				{
					if (!readNulField(raw, pos, fe.path))
						break;
					if (sc == 'A')
						fe.kind = ChangeKind::Added;
					else if (sc == 'D')
						fe.kind = ChangeKind::Deleted;
					else
						fe.kind = ChangeKind::Modified;
				}

				if (!fe.path.empty())
					byPath[fe.path] = std::move(fe);
			}
		}

		/// Parses `git ls-files -z` output (NUL-separated paths).
		void parseNulPaths(const std::string &raw, std::unordered_set<std::string> &out)
		{
			size_t pos = 0;
			while (pos < raw.size())
			{
				std::string p;
				if (!readNulField(raw, pos, p))
					break;
				if (!p.empty())
					out.insert(std::move(p));
			}
		}

		// ── Binary detection helpers ─────────────────────────────────

		std::string toLowerExt(const std::string &path)
		{
			const auto dot = path.rfind('.');
			if (dot == std::string::npos)
				return {};
			std::string ext = path.substr(dot);
			for (char &c : ext)
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			return ext;
		}
	}

	bool pathLooksBinaryByContent(const std::string &text)
	{
		if (text.empty())
			return false;
		const size_t checkLen = std::min(text.size(), static_cast<size_t>(8192));
		size_t nonPrintable = 0;
		for (size_t i = 0; i < checkLen; i++)
		{
			const unsigned char c = static_cast<unsigned char>(text[i]);
			if (c == '\0')
				return true;
			if (c < 0x08)
				nonPrintable++;
			else if (c >= 0x0E && c < 0x20 && c != 0x1B)
				nonPrintable++;
		}
		return checkLen > 0 && nonPrintable * 10 > checkLen;
	}

	bool pathLooksBinaryByExtension(const std::string &path)
	{
		const std::string ext = toLowerExt(path);
		if (ext.empty())
			return false;
		static const char *const binaryExts[] = {
			".png", ".jpg",  ".jpeg", ".gif",  ".bmp",  ".ico",  ".webp",  ".tiff", ".tif",  ".mp3",  ".mp4",  ".wav",
			".flac", ".ogg", ".avi",  ".mkv",  ".mov",  ".wmv",  ".zip",   ".gz",   ".bz2",  ".xz",   ".7z",   ".rar",  ".tar",
			".exe",  ".dll", ".so",   ".dylib", ".o",   ".obj",  ".a",     ".lib",  ".pdf",  ".doc",  ".docx", ".xls",  ".xlsx",
			".ppt",  ".pptx", ".class", ".pyc", ".pyo", ".ttf",  ".otf",   ".woff", ".woff2", ".eot", ".sqlite", ".db", ".wasm",
		};
		for (const char *e : binaryExts)
		{
			if (ext == e)
				return true;
		}
		return false;
	}

	bool pathLooksTextByExtension(const std::string &path)
	{
		const std::string ext = toLowerExt(path);
		if (ext.empty())
			return false;
		static const char *const textExts[] = {
			".txt",  ".md",   ".markdown", ".rst",  ".adoc", ".tex",  ".svg",  ".xml",  ".html", ".htm", ".xhtml",
			".css",  ".scss", ".less",    ".sass",
			".json", ".jsonc", ".json5", ".yaml", ".yml", ".toml", ".ini",  ".cfg",  ".conf", ".properties",
			".c",    ".h",    ".cc",     ".cpp",  ".cxx",  ".hpp",  ".hh",   ".hxx",  ".m",    ".mm",
			".cs",   ".fs",   ".fsx",    ".fsi",  ".vb",   ".rs",   ".go",   ".java", ".kt",   ".kts",
			".py",   ".pyw",  ".pyi",    ".rb",   ".php",  ".swift", ".scala", ".sc",  ".pl",   ".pm",  ".lua", ".vim",
			".js",   ".jsx",  ".mjs",    ".cjs",  ".ts",   ".tsx",  ".vue",  ".svelte",
			".sh",   ".bash", ".zsh",    ".fish", ".ps1",  ".bat",  ".cmd",
			".sql",  ".graphql", ".gql", ".cmake",
			".glsl", ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese",
			".hlsl", ".metal",
			".csv",  ".tsv",  ".gitignore", ".gitattributes", ".editorconfig", ".dockerignore",
		};
		for (const char *e : textExts)
		{
			if (ext == e)
				return true;
		}
		return false;
	}

	static void decideBinaryAfterLoad(const FileEntry &entry, const std::string &left, const std::string &right, bool &outBinary)
	{
		const bool lc = pathLooksBinaryByContent(left);
		const bool rc = pathLooksBinaryByContent(right);
		const bool extBin = pathLooksBinaryByExtension(entry.path) || (!entry.renameFrom.empty() && pathLooksBinaryByExtension(entry.renameFrom));
		const bool textHint = pathLooksTextByExtension(entry.path) || (!entry.renameFrom.empty() && pathLooksTextByExtension(entry.renameFrom));
		if (textHint && !lc && !rc)
			outBinary = false;
		else
			outBinary = lc || rc || (extBin && !textHint);
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

		outSnap.commitsAheadOfUpstream = -1;
		GitRunResult ahead = runGit(repoRoot, { "rev-list", "--count", "@{upstream}..HEAD" });
		if (ahead.exitCode == 0)
		{
			const std::string t = trim(ahead.standardOut);
			if (!t.empty())
			{
				char *endPtr = nullptr;
				const long n = std::strtol(t.c_str(), &endPtr, 10);
				if (endPtr != t.c_str() && n >= 0 && n <= static_cast<long>(INT_MAX))
					outSnap.commitsAheadOfUpstream = static_cast<int>(n);
			}
		}

		GitRunResult unst = runGit(repoRoot, { "diff", "--find-renames", "--name-status", "-z" });
		GitRunResult stgd = runGit(repoRoot, { "diff", "--cached", "--find-renames", "--name-status", "-z" });
		GitRunResult untr = runGit(repoRoot, { "ls-files", "-o", "--exclude-standard", "-z" });

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
		appendNameStatusNul(staged, stgd.standardOut, FileListSection::Staged);
		appendNameStatusNul(unstaged, unst.standardOut, FileListSection::Unstaged);

		std::unordered_set<std::string> untrackedPaths;
		parseNulPaths(untr.standardOut, untrackedPaths);

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

		for (auto &kv : unstaged)
			outSnap.entries.push_back(std::move(kv.second));

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

		// `git show REV:path` — the path must be repo-relative.
		// The paths stored in FileEntry come from git's own output and are
		// already repo-relative, so they are safe to concatenate directly.
		// We validate non-emptiness as a guard against logic bugs.
		auto showObject = [&](const std::string &prefix, const std::string &pathInRepo, std::string &into) {
			if (pathInRepo.empty())
			{
				into.clear();
				return false;
			}
			GitRunResult r = runGit(repoRoot, { "show", prefix + pathInRepo });
			if (r.exitCode != 0)
			{
				into.clear();
				return false;
			}
			into = std::move(r.standardOut);
			return true;
		};

		auto showHeadPath = [&](const std::string &pathInRepo, std::string &into) { return showObject("HEAD:", pathInRepo, into); };
		auto showIndexPath = [&](const std::string &pathInRepo, std::string &into) { return showObject(":", pathInRepo, into); };

		if (entry.section == FileListSection::Untracked)
		{
			outLeft.clear();
			outRight = readFileUtf8(workPath.string(), outError);
			outRightIsWorktreeFile = true;
			if (!outError.empty())
				return;
			decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
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
				decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
				return;
			}

			if (entry.kind == ChangeKind::Renamed)
			{
				if (!showIndexPath(entry.renameFrom, outLeft))
				{
					outError = "Could not read old path at index for rename.";
					return;
				}
				outRight = readFileUtf8(workPath.string(), outError);
				outRightIsWorktreeFile = true;
				if (!outError.empty())
					return;
				decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
				return;
			}

			if (entry.kind == ChangeKind::Added)
			{
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
			decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
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
			decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
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
			decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
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
			decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
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
		decideBinaryAfterLoad(entry, outLeft, outRight, outBinary);
	}
}
