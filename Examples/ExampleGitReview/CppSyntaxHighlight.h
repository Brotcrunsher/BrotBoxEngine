#pragma once

#include "imgui.h"

#include <string>

namespace gitReview
{
	bool pathLooksLikeCpp(const std::string &path);

	/// Draws one line of C++-ish highlighting at \p lineTextPos (top-left of the text, before horizontal scroll).
	void drawCppSyntaxLineOverlay(ImDrawList *dl, ImFont *font, float fontSize, const ImVec2 &lineTextPos, const std::string &line,
		float scrollXPixels, const ImVec2 &clipMin, const ImVec2 &clipMax, ImU32 defaultTextCol);
}
