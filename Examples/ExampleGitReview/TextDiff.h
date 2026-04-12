#pragma once

#include <string>
#include <vector>

namespace gitReview
{
	enum class DiffRowKind
	{
		Equal,
		/// One line removed on the left (blank right row).
		LeftOnly,
		/// One line added on the right (blank left row).
		RightOnly,
		/// Neighboring delete+insert collapsed into one row for intra-line highlighting.
		Changed,
	};

	struct DiffRow
	{
		DiffRowKind kind = DiffRowKind::Equal;
		std::string leftLine;
		std::string rightLine;
	};

	enum class WordSpanKind
	{
		Stable,
		Removed,
		Added,
	};

	struct WordSpan
	{
		WordSpanKind kind = WordSpanKind::Stable;
		std::string text;
	};

	/// Splits text into lines (LF / CRLF); omits a trailing empty line caused by a final newline.
	std::vector<std::string> splitLinesForDiff(const std::string &text);

	/// LCS-based (dynamic-programming) line diff; merges adjacent
	/// delete+insert pairs into \c DiffRowKind::Changed for intra-line
	/// highlighting.  When the grid would be too large, falls back to
	/// a line-level display without LCS alignment and sets
	/// \p outLargeFallback.
	std::vector<DiffRow> buildSideBySideRows(const std::string &leftText, const std::string &rightText, bool &outLargeFallback);

	/// Word-level highlighting for one left/right line pair (whitespace-separated tokens).
	void buildWordSpans(const std::string &leftLine, const std::string &rightLine, std::vector<WordSpan> &outLeft,
		std::vector<WordSpan> &outRight);

	/// Inverse of \c splitLinesForDiff for round-tripping line vectors.
	std::string joinLinesForDiff(const std::vector<std::string> &lines);

	/// Reconstructs on-disk text from a side-by-side row-aligned buffer (one physical line per \c DiffRow).
	std::string canonicalFromAlignedRightBuffer(const std::vector<std::string> &alignedLines, const std::vector<DiffRow> &rows);

	/// Builds a row-aligned buffer so each diff row maps to one editor line (empty line for \c LeftOnly rows).
	std::string buildAlignedRightBuffer(const std::string &canonicalRight, const std::vector<DiffRow> &rows);

	/// Row-aligned left buffer: empty line for \c RightOnly rows; otherwise \c leftLine from each row.
	std::string buildAlignedLeftBuffer(const std::vector<DiffRow> &rows);

	/// One row in a merge-compare grid: index stages aligned to the same working-tree line where possible.
	struct MergeThreePaneRow
	{
		std::string baseLine;
		std::string oursLine;
		std::string theirsLine;
		std::string workLine;
		/// 0-based physical line in \p work text used for conflict-marker mapping, or \c -1 when \c workLine is empty.
		int workLineIndex0 = -1;
	};

	/// Builds rows for base / ours / theirs vs. the working tree, padded to a common length. \p outLargeFallback
	/// is true if any of the underlying pairwise diffs used the large-file fallback.
	std::vector<MergeThreePaneRow> buildMergeThreePaneRows(const std::string &base, const std::string &ours, const std::string &work,
		const std::string &theirs, bool &outLargeFallback);

	/// Joins \c workLine fields (same convention as \c joinLinesForDiff).
	std::string mergeRowsWorkCanon(const std::vector<MergeThreePaneRow> &rows);

	enum class MergePaneColumn
	{
		Base,
		Ours,
		Theirs,
		Work,
	};

	/// One physical line per merge row (for multiline panes).
	std::string buildAlignedMergePaneBuffer(const std::vector<MergeThreePaneRow> &rows, MergePaneColumn which);
}
