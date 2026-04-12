#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace gitReview
{
	enum class MergeConflictPick
	{
		Ours,
		Theirs,
		OursThenTheirs,
		TheirsThenOurs,
	};

	/// One Git-style conflict block: `<<<<<<<` … `=======` … `>>>>>>>`.
	struct MergeConflictHunk
	{
		/// Byte offset of the first character of the line containing `<<<<<<<`.
		size_t openLineBegin = 0;
		/// Byte offset of the first character of the line containing `=======`.
		size_t sepLineBegin = 0;
		/// Byte offset of the first character of the line containing `>>>>>>>`.
		size_t closeLineBegin = 0;
		/// First byte after the `>>>>>>>` line (past the newline, or end of string).
		size_t afterClose = 0;
	};

	std::vector<MergeConflictHunk> listMergeConflictHunks(std::string_view doc);

	/// Replaces hunk \p hunkIndex (0-based in \c listMergeConflictHunks order) using \p pick. Returns false on failure.
	bool applyMergeConflictPick(std::string &doc, size_t hunkIndex, MergeConflictPick pick, std::string *outError);

	/// 0-based inclusive line range in \p doc (split by `\n`) covered by each conflict hunk (including marker lines).
	struct MergeConflictHunkLines
	{
		size_t hunkIndex = 0;
		int firstLine0 = 0;
		int lastLine0 = 0;
	};

	std::vector<MergeConflictHunkLines> listMergeConflictHunkLines(const std::string &doc);

	/// Returns hunk index, or \c -1 if none.
	int mergeConflictHunkAtLine(const std::vector<MergeConflictHunkLines> &ranges, int line0);

}
