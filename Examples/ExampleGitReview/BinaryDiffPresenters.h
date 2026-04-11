#pragma once

#include <string>

namespace gitReview
{
	/// Side-by-side hex, image thumbnails (when decodable), and magic-byte summaries for non-text files.
	void drawBinaryDiffPresenters(const std::string &pathHint, const std::string &leftBytes, const std::string &rightBytes);
}
