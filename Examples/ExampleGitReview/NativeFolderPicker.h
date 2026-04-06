#pragma once

#include <optional>
#include <string>

namespace gitReview
{
	/// Native folder picker where available; returns nullopt if cancelled or unavailable.
	std::optional<std::string> pickFolderInteractive(const char *title);
}
