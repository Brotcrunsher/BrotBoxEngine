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
}
