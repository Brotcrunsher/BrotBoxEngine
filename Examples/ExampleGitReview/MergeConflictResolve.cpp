#include "MergeConflictResolve.h"

namespace gitReview
{
	namespace
	{
		void trimTrailingCr(std::string &s)
		{
			while (!s.empty() && s.back() == '\r')
				s.pop_back();
		}

		bool lineStartsWithMarker(std::string_view line, std::string_view marker)
		{
			while (!line.empty() && line.front() == ' ')
				line.remove_prefix(1);
			return line.size() >= marker.size() && line.compare(0, marker.size(), marker) == 0;
		}

		/// First index >= \p from at a line start where the line begins with \p marker (after optional spaces / CR).
		size_t findMarkerLine(std::string_view doc, size_t from, std::string_view marker)
		{
			size_t i = from;
			while (i < doc.size())
			{
				const size_t lineEnd = doc.find('\n', i);
				const std::string_view line = doc.substr(i, (lineEnd == std::string::npos ? doc.size() : lineEnd) - i);
				if (lineStartsWithMarker(line, marker))
					return i;
				if (lineEnd == std::string::npos)
					break;
				i = lineEnd + 1;
			}
			return std::string::npos;
		}

		/// Byte offset of the first character after the line that starts at \p lineBegin (past `\n`, or `doc.size()`).
		size_t pastLine(std::string_view doc, size_t lineBegin)
		{
			const size_t lineEnd = doc.find('\n', lineBegin);
			if (lineEnd == std::string::npos)
				return doc.size();
			return lineEnd + 1;
		}

		std::string concatBlocks(const std::string &a, const std::string &b)
		{
			if (a.empty())
				return b;
			if (b.empty())
				return a;
			if (a.back() == '\n')
				return a + b;
			return a + '\n' + b;
		}
	} // namespace

	std::vector<MergeConflictHunk> listMergeConflictHunks(std::string_view doc)
	{
		static constexpr std::string_view kOpen = "<<<<<<<";
		static constexpr std::string_view kMid = "=======";
		static constexpr std::string_view kClose = ">>>>>>>";

		std::vector<MergeConflictHunk> out;
		size_t search = 0;
		for (;;)
		{
			const size_t p = doc.find("<<<<<<<", search);
			if (p == std::string::npos)
				break;
			if (p > 0 && doc[p - 1] != '\n')
			{
				search = p + 7;
				continue;
			}
			const size_t openLineBegin = p;

			const size_t oursRegionStart = pastLine(doc, openLineBegin);
			if (oursRegionStart > doc.size())
				break;

			const size_t sepLineBegin = findMarkerLine(doc, oursRegionStart, kMid);
			if (sepLineBegin == std::string::npos)
				break;

			const size_t theirsRegionStart = pastLine(doc, sepLineBegin);
			if (theirsRegionStart > doc.size())
				break;

			const size_t closeLineBegin = findMarkerLine(doc, theirsRegionStart, kClose);
			if (closeLineBegin == std::string::npos)
				break;

			MergeConflictHunk h;
			h.openLineBegin = openLineBegin;
			h.sepLineBegin = sepLineBegin;
			h.closeLineBegin = closeLineBegin;
			h.afterClose = pastLine(doc, closeLineBegin);
			out.push_back(h);
			search = h.afterClose;
		}
		return out;
	}

	static int byteOffsetToLine0(std::string_view doc, size_t byteOff)
	{
		if (byteOff > doc.size())
			byteOff = doc.size();
		int ln = 0;
		for (size_t i = 0; i < byteOff; ++i)
		{
			if (doc[i] == '\n')
				++ln;
		}
		return ln;
	}

	std::vector<MergeConflictHunkLines> listMergeConflictHunkLines(const std::string &doc)
	{
		const std::vector<MergeConflictHunk> hunks = listMergeConflictHunks(doc);
		std::vector<MergeConflictHunkLines> out;
		out.reserve(hunks.size());
		for (size_t i = 0; i < hunks.size(); ++i)
		{
			const MergeConflictHunk &h = hunks[i];
			MergeConflictHunkLines hl;
			hl.hunkIndex = i;
			hl.firstLine0 = byteOffsetToLine0(doc, h.openLineBegin);
			const size_t lastByte = h.afterClose > 0 ? h.afterClose - 1 : h.openLineBegin;
			hl.lastLine0 = byteOffsetToLine0(doc, lastByte);
			out.push_back(hl);
		}
		return out;
	}

	int mergeConflictHunkAtLine(const std::vector<MergeConflictHunkLines> &ranges, int line0)
	{
		for (size_t i = 0; i < ranges.size(); ++i)
		{
			if (line0 >= ranges[i].firstLine0 && line0 <= ranges[i].lastLine0)
				return static_cast<int>(i);
		}
		return -1;
	}

	bool applyMergeConflictPick(std::string &doc, size_t hunkIndex, MergeConflictPick pick, std::string *outError)
	{
		if (outError)
			outError->clear();
		std::vector<MergeConflictHunk> hunks = listMergeConflictHunks(doc);
		if (hunkIndex >= hunks.size())
		{
			if (outError)
				*outError = "Conflict hunk index out of range.";
			return false;
		}
		const MergeConflictHunk &h = hunks[hunkIndex];

		const size_t oursStart = pastLine(doc, h.openLineBegin);
		if (oursStart > h.sepLineBegin)
		{
			if (outError)
				*outError = "Malformed conflict (ours region).";
			return false;
		}
		const size_t theirsStart = pastLine(doc, h.sepLineBegin);
		if (theirsStart > h.closeLineBegin)
		{
			if (outError)
				*outError = "Malformed conflict (theirs region).";
			return false;
		}

		std::string ours(doc.substr(oursStart, h.sepLineBegin - oursStart));
		std::string theirs(doc.substr(theirsStart, h.closeLineBegin - theirsStart));
		trimTrailingCr(ours);
		trimTrailingCr(theirs);

		std::string replacement;
		switch (pick)
		{
		case MergeConflictPick::Ours:
			replacement = ours;
			break;
		case MergeConflictPick::Theirs:
			replacement = theirs;
			break;
		case MergeConflictPick::OursThenTheirs:
			replacement = concatBlocks(ours, theirs);
			break;
		case MergeConflictPick::TheirsThenOurs:
			replacement = concatBlocks(theirs, ours);
			break;
		}

		doc.replace(h.openLineBegin, h.afterClose - h.openLineBegin, replacement);
		return true;
	}
}
