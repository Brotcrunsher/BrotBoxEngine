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
}
