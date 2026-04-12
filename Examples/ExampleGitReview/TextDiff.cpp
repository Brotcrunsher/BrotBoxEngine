#include "TextDiff.h"

#include <algorithm>
#include <cctype>
namespace gitReview
{
	namespace
	{
		bool isWhitespaceChar(char c)
		{
			return static_cast<unsigned char>(c) <= ' ' && c != '\0';
		}

	}

	std::vector<std::string> splitLinesForDiff(const std::string &text)
	{
		std::vector<std::string> lines;
		size_t pos = 0;
		while (pos < text.size())
		{
			const size_t nl = text.find('\n', pos);
			if (nl == std::string::npos)
			{
				lines.push_back(text.substr(pos));
				break;
			}
			std::string line = text.substr(pos, nl - pos);
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			lines.push_back(std::move(line));
			pos = nl + 1;
		}
		return lines;
	}

	namespace
	{
		enum class Edit : unsigned char
		{
			Match,
			Delete,
			Insert,
		};

		/// Myers' diff algorithm – O((n+m)*d) where d = edit distance.
		/// Falls back (returns false) when d exceeds the budget,
		/// signalling the caller to use the large-diff fallback.
		bool lineDiffScript(const std::vector<std::string> &a, const std::vector<std::string> &b, std::vector<Edit> &script)
		{
			script.clear();
			const int n = static_cast<int>(a.size());
			const int m = static_cast<int>(b.size());
			if (n == 0 && m == 0)
				return true;
			if (n == 0)
			{
				script.assign(static_cast<size_t>(m), Edit::Insert);
				return true;
			}
			if (m == 0)
			{
				script.assign(static_cast<size_t>(n), Edit::Delete);
				return true;
			}

			const int budget = std::min(n + m, 2000);
			const int vOffset = budget;
			const int vSize = 2 * budget + 1;

			std::vector<std::vector<int>> traces;
			std::vector<int> v(static_cast<size_t>(vSize), 0);

			int finalD = -1;
			for (int d = 0; d <= budget; d++)
			{
				traces.emplace_back(v.begin() + vOffset - d, v.begin() + vOffset + d + 1);

				for (int k = -d; k <= d; k += 2)
				{
					int x;
					if (k == -d || (k != d && v[k - 1 + vOffset] < v[k + 1 + vOffset]))
						x = v[k + 1 + vOffset];
					else
						x = v[k - 1 + vOffset] + 1;

					int y = x - k;
					while (x < n && y < m && a[x] == b[y])
					{
						x++;
						y++;
					}
					v[k + vOffset] = x;

					if (x >= n && y >= m)
					{
						finalD = d;
						break;
					}
				}
				if (finalD >= 0)
					break;
			}

			if (finalD < 0)
				return false;

			int x = n, y = m;
			std::vector<Edit> rev;

			for (int d = finalD; d > 0; d--)
			{
				const int k = x - y;
				const auto &tv = traces[static_cast<size_t>(d)];
				auto tvAt = [&](int kk) { return tv[static_cast<size_t>(kk + d)]; };

				int prevK;
				if (k == -d || (k != d && tvAt(k - 1) < tvAt(k + 1)))
					prevK = k + 1;
				else
					prevK = k - 1;

				const int prevX = tvAt(prevK);
				const int prevY = prevX - prevK;

				while (x > prevX && y > prevY)
				{
					rev.push_back(Edit::Match);
					x--;
					y--;
				}

				rev.push_back(k == prevK + 1 ? Edit::Delete : Edit::Insert);
				x = prevX;
				y = prevY;
			}

			while (x > 0 && y > 0)
			{
				rev.push_back(Edit::Match);
				x--;
				y--;
			}

			std::reverse(rev.begin(), rev.end());
			script = std::move(rev);
			return true;
		}

		/// Fallback for diffs that exceed the DP cell budget.
		/// Pairs up lines positionally: equal lines → Equal,
		/// differing lines → Changed, and excess lines on either
		/// side → LeftOnly / RightOnly.  No word-level diff is
		/// attempted for Changed rows in the renderer when the
		/// large-fallback flag is set.
		std::vector<DiffRow> buildLargeFallback(const std::vector<std::string> &a, const std::vector<std::string> &b)
		{
			std::vector<DiffRow> rows;
			const size_t common = std::min(a.size(), b.size());
			rows.reserve(std::max(a.size(), b.size()));

			for (size_t i = 0; i < common; i++)
			{
				DiffRow r;
				if (a[i] == b[i])
				{
					r.kind = DiffRowKind::Equal;
					r.leftLine = a[i];
					r.rightLine = b[i];
				}
				else
				{
					r.kind = DiffRowKind::Changed;
					r.leftLine = a[i];
					r.rightLine = b[i];
				}
				rows.push_back(std::move(r));
			}
			for (size_t i = common; i < a.size(); i++)
			{
				DiffRow r;
				r.kind = DiffRowKind::LeftOnly;
				r.leftLine = a[i];
				rows.push_back(std::move(r));
			}
			for (size_t i = common; i < b.size(); i++)
			{
				DiffRow r;
				r.kind = DiffRowKind::RightOnly;
				r.rightLine = b[i];
				rows.push_back(std::move(r));
			}
			return rows;
		}
	}

	std::vector<DiffRow> buildSideBySideRows(const std::string &leftText, const std::string &rightText, bool &outLargeFallback)
	{
		outLargeFallback = false;
		const std::vector<std::string> a = splitLinesForDiff(leftText);
		const std::vector<std::string> b = splitLinesForDiff(rightText);
		std::vector<Edit> script;
		if (!lineDiffScript(a, b, script))
		{
			outLargeFallback = true;
			return buildLargeFallback(a, b);
		}

		std::vector<DiffRow> rows;
		size_t i = 0, j = 0;
		for (Edit e : script)
		{
			if (e == Edit::Match)
			{
				DiffRow r;
				r.kind = DiffRowKind::Equal;
				r.leftLine = a[i++];
				r.rightLine = b[j++];
				rows.push_back(std::move(r));
			}
			else if (e == Edit::Delete)
			{
				DiffRow r;
				r.kind = DiffRowKind::LeftOnly;
				r.leftLine = a[i++];
				rows.push_back(std::move(r));
			}
			else
			{
				DiffRow r;
				r.kind = DiffRowKind::RightOnly;
				r.rightLine = b[j++];
				rows.push_back(std::move(r));
			}
		}

		for (size_t k = 0; k + 1 < rows.size(); k++)
		{
			if (rows[k].kind == DiffRowKind::LeftOnly && rows[k + 1].kind == DiffRowKind::RightOnly)
			{
				rows[k].kind = DiffRowKind::Changed;
				rows[k].rightLine = std::move(rows[k + 1].rightLine);
				rows.erase(rows.begin() + static_cast<std::ptrdiff_t>(k + 1));
			}
		}
		return rows;
	}

	namespace
	{
		void tokenizeMixed(const std::string &line, std::vector<std::string> &outTokens)
		{
			outTokens.clear();
			size_t i = 0;
			const size_t n = line.size();
			while (i < n)
			{
				const bool ws = isWhitespaceChar(line[i]);
				size_t j = i + 1;
				while (j < n && isWhitespaceChar(line[j]) == ws)
					j++;
				outTokens.push_back(line.substr(i, j - i));
				i = j;
			}
		}

		void lcsTable(const std::vector<std::string> &a, const std::vector<std::string> &b, std::vector<int> &table, int &cols)
		{
			const int na = static_cast<int>(a.size());
			const int nb = static_cast<int>(b.size());
			cols = nb + 1;
			table.assign(static_cast<size_t>((na + 1) * (nb + 1)), 0);
			for (int i = na - 1; i >= 0; i--)
			{
				for (int j = nb - 1; j >= 0; j--)
				{
					const int cur = i * cols + j;
					if (a[static_cast<size_t>(i)] == b[static_cast<size_t>(j)])
						table[static_cast<size_t>(cur)] = 1 + table[static_cast<size_t>((i + 1) * cols + (j + 1))];
					else
						table[static_cast<size_t>(cur)] = std::max(table[static_cast<size_t>((i + 1) * cols + j)], table[static_cast<size_t>(i * cols + (j + 1))]);
				}
			}
		}
	}

	std::string joinLinesForDiff(const std::vector<std::string> &lines)
	{
		std::string o;
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (i)
				o += '\n';
			o += lines[i];
		}
		return o;
	}

	std::string canonicalFromAlignedRightBuffer(const std::vector<std::string> &alignedLines, const std::vector<DiffRow> &rows)
	{
		std::vector<std::string> canonLines;
		const size_t n = std::min(alignedLines.size(), rows.size());
		canonLines.reserve(n);
		for (size_t i = 0; i < n; i++)
		{
			if (rows[i].kind == DiffRowKind::LeftOnly)
			{
				if (!alignedLines[i].empty())
					canonLines.push_back(alignedLines[i]);
				continue;
			}
			canonLines.push_back(alignedLines[i]);
		}
		return joinLinesForDiff(canonLines);
	}

	std::string buildAlignedRightBuffer(const std::string &canonicalRight, const std::vector<DiffRow> &rows)
	{
		const std::vector<std::string> L = splitLinesForDiff(canonicalRight);
		std::vector<std::string> outLines;
		outLines.reserve(rows.size());
		size_t li = 0;
		for (const DiffRow &r : rows)
		{
			if (r.kind == DiffRowKind::LeftOnly)
			{
				outLines.emplace_back();
				continue;
			}
			if (li < L.size())
				outLines.push_back(L[li++]);
			else
				outLines.emplace_back();
		}
		return joinLinesForDiff(outLines);
	}

	std::string buildAlignedLeftBuffer(const std::vector<DiffRow> &rows)
	{
		std::vector<std::string> outLines;
		outLines.reserve(rows.size());
		for (const DiffRow &r : rows)
		{
			if (r.kind == DiffRowKind::RightOnly)
				outLines.emplace_back();
			else
				outLines.push_back(r.leftLine);
		}
		return joinLinesForDiff(outLines);
	}

	void buildWordSpans(const std::string &leftLine, const std::string &rightLine, std::vector<WordSpan> &outLeft,
		std::vector<WordSpan> &outRight)
	{
		outLeft.clear();
		outRight.clear();
		std::vector<std::string> a, b;
		tokenizeMixed(leftLine, a);
		tokenizeMixed(rightLine, b);
		if (a.empty() && b.empty())
			return;

		const long long cells = static_cast<long long>(a.size() + 1) * static_cast<long long>(b.size() + 1);
		if (cells > 400'000)
		{
			WordSpan l;
			l.kind = WordSpanKind::Removed;
			l.text = leftLine;
			outLeft.push_back(l);
			WordSpan r;
			r.kind = WordSpanKind::Added;
			r.text = rightLine;
			outRight.push_back(r);
			return;
		}

		std::vector<int> table;
		int cols = 0;
		lcsTable(a, b, table, cols);

		size_t i = 0, j = 0;
		while (i < a.size() || j < b.size())
		{
			const int ii = static_cast<int>(i);
			const int jj = static_cast<int>(j);
			if (i < a.size() && j < b.size() && a[i] == b[j])
			{
				WordSpan sl;
				sl.kind = WordSpanKind::Stable;
				sl.text = a[i];
				outLeft.push_back(sl);
				WordSpan sr;
				sr.kind = WordSpanKind::Stable;
				sr.text = b[j];
				outRight.push_back(sr);
				i++;
				j++;
				continue;
			}
			const int down = (i < a.size()) ? table[static_cast<size_t>((ii + 1) * cols + jj)] : -1;
			const int right = (j < b.size()) ? table[static_cast<size_t>(ii * cols + (jj + 1))] : -1;
			if (j >= b.size() || (i < a.size() && down >= right))
			{
				const bool tokenIsWs = !a[i].empty() && isWhitespaceChar(a[i][0]);
				WordSpan sl;
				sl.kind = tokenIsWs ? WordSpanKind::Stable : WordSpanKind::Removed;
				sl.text = a[i];
				outLeft.push_back(sl);
				i++;
			}
			else
			{
				const bool tokenIsWs = !b[j].empty() && isWhitespaceChar(b[j][0]);
				WordSpan sr;
				sr.kind = tokenIsWs ? WordSpanKind::Stable : WordSpanKind::Added;
				sr.text = b[j];
				outRight.push_back(sr);
				j++;
			}
		}
	}

	std::vector<MergeThreePaneRow> buildMergeThreePaneRows(const std::string &base, const std::string &ours, const std::string &work,
		const std::string &theirs, bool &outLargeFallback)
	{
		outLargeFallback = false;
		bool l1 = false, l2 = false, l3 = false;
		const std::vector<DiffRow> a = buildSideBySideRows(ours, work, l1);
		const std::vector<DiffRow> b = buildSideBySideRows(theirs, work, l2);
		const std::vector<DiffRow> c = buildSideBySideRows(base, work, l3);
		outLargeFallback = l1 || l2 || l3;
		const size_t L = std::max({a.size(), b.size(), c.size()});
		std::vector<MergeThreePaneRow> rows(L);
		for (size_t i = 0; i < L; ++i)
		{
			MergeThreePaneRow &row = rows[i];
			row.oursLine = i < a.size() ? a[i].leftLine : std::string{};
			row.theirsLine = i < b.size() ? b[i].leftLine : std::string{};
			row.baseLine = i < c.size() ? c[i].leftLine : std::string{};
			std::string w;
			if (i < a.size() && !a[i].rightLine.empty())
				w = a[i].rightLine;
			else if (i < b.size() && !b[i].rightLine.empty())
				w = b[i].rightLine;
			else if (i < c.size() && !c[i].rightLine.empty())
				w = c[i].rightLine;
			row.workLine = std::move(w);
		}

		const std::vector<std::string> wl = splitLinesForDiff(work);
		size_t cursor = 0;
		for (MergeThreePaneRow &row : rows)
		{
			const std::string &w = row.workLine;
			if (w.empty())
			{
				row.workLineIndex0 = -1;
				continue;
			}
			int found = -1;
			for (size_t j = cursor; j < wl.size(); ++j)
			{
				if (wl[j] == w)
				{
					found = static_cast<int>(j);
					cursor = j + 1;
					break;
				}
			}
			if (found < 0)
			{
				for (size_t j = 0; j < wl.size(); ++j)
				{
					if (wl[j] == w)
					{
						found = static_cast<int>(j);
						cursor = j + 1;
						break;
					}
				}
			}
			row.workLineIndex0 = found;
		}
		return rows;
	}

	std::string mergeRowsWorkCanon(const std::vector<MergeThreePaneRow> &rows)
	{
		std::vector<std::string> lines;
		lines.reserve(rows.size());
		for (const MergeThreePaneRow &r : rows)
			lines.push_back(r.workLine);
		return joinLinesForDiff(lines);
	}

	std::string buildAlignedMergePaneBuffer(const std::vector<MergeThreePaneRow> &rows, MergePaneColumn which)
	{
		std::vector<std::string> lines;
		lines.reserve(rows.size());
		for (const MergeThreePaneRow &r : rows)
		{
			switch (which)
			{
			case MergePaneColumn::Base:
				lines.push_back(r.baseLine);
				break;
			case MergePaneColumn::Ours:
				lines.push_back(r.oursLine);
				break;
			case MergePaneColumn::Theirs:
				lines.push_back(r.theirsLine);
				break;
			case MergePaneColumn::Work:
				lines.push_back(r.workLine);
				break;
			}
		}
		return joinLinesForDiff(lines);
	}
}
