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
}
