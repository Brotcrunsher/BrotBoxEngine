#include "CppSyntaxHighlight.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string_view>

namespace gitReview
{
	namespace
	{
		static bool asciiIsSpace(char c)
		{
			return c == ' ' || c == '\t' || c == '\r';
		}

		static bool asciiIdentStart(char c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
		}

		static bool asciiIdentCont(char c)
		{
			return asciiIdentStart(c) || (c >= '0' && c <= '9');
		}

		/// Lexicographically sorted for binary search.
		static const char *const kCppKeywords[] = {
			"alignas", "alignof", "asm", "auto", "bool", "break", "case", "catch", "char", "class", "co_await", "co_return", "co_yield", "concept",
			"const", "const_cast", "consteval", "constexpr", "constinit", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast",
			"else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "int16_t", "int32_t",
			"int64_t", "int8_t", "long", "mutable", "namespace", "new", "noexcept", "nullptr", "operator", "private", "protected", "public", "register",
			"reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "size_t", "ssize_t", "static", "static_assert", "static_cast", "struct",
			"switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "uint16_t", "uint32_t", "uint64_t",
			"uint8_t", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while",
		};

		static bool isCppKeyword(std::string_view id)
		{
			const auto *first = std::begin(kCppKeywords);
			const auto *last = std::end(kCppKeywords);
			auto it = std::lower_bound(first, last, id,
				[](const char *kw, std::string_view v) { return std::string_view(kw) < v; });
			return it != last && std::string_view(*it) == id;
		}

		static bool onlySpacesBefore(const char *lineBegin, const char *p)
		{
			for (const char *q = lineBegin; q < p; ++q)
			{
				if (!asciiIsSpace(*q))
					return false;
			}
			return true;
		}

		static ImU32 hlKeyword()
		{
			return IM_COL32(86, 156, 214, 255);
		}
		static ImU32 hlNumber()
		{
			return IM_COL32(181, 206, 168, 255);
		}
		static ImU32 hlString()
		{
			return IM_COL32(206, 145, 120, 255);
		}
		static ImU32 hlComment()
		{
			return IM_COL32(106, 153, 85, 255);
		}
		static ImU32 hlPreproc()
		{
			return IM_COL32(197, 134, 192, 255);
		}
	} // namespace

	bool pathLooksLikeCpp(const std::string &path)
	{
		static const char *exts[] = {".cpp", ".cxx", ".cc", ".c++", ".h", ".hh", ".hpp", ".hxx", ".inl", ".ipp", ".tpp"};
		for (const char *ext : exts)
		{
			const size_t el = std::strlen(ext);
			if (path.size() >= el && std::memcmp(path.c_str() + path.size() - el, ext, el) == 0)
				return true;
		}
		return false;
	}

	void drawCppSyntaxLineOverlay(ImDrawList *dl, ImFont *font, float fontSize, const ImVec2 &lineTextPos, const std::string &line,
		float scrollXPixels, const ImVec2 &clipMin, const ImVec2 &clipMax, ImU32 defaultTextCol)
	{
		if (line.empty())
			return;

		const char *const lineBegin = line.data();
		const char *const lineEnd = lineBegin + line.size();
		const ImVec4 clip4(clipMin.x, clipMin.y, clipMax.x, clipMax.y);

		float x = lineTextPos.x - scrollXPixels;
		const float y = lineTextPos.y;
		const char *p = lineBegin;

		auto emit = [&](const char *a, const char *b, ImU32 col) {
			if (a >= b)
				return;
			dl->AddText(font, fontSize, ImVec2(x, y), col, a, b, 0.f, &clip4);
			x += ImGui::CalcTextSize(a, b).x;
		};

		while (p < lineEnd)
		{
			if (asciiIsSpace(*p))
			{
				const char *ws = p;
				while (p < lineEnd && asciiIsSpace(*p))
					++p;
				emit(ws, p, defaultTextCol);
				continue;
			}

			if (p[0] == '/' && p + 1 < lineEnd && p[1] == '/')
			{
				emit(p, lineEnd, hlComment());
				break;
			}

			if (p[0] == '/' && p + 1 < lineEnd && p[1] == '*')
			{
				const char *q = p + 2;
				while (q + 1 < lineEnd && !(q[0] == '*' && q[1] == '/'))
					++q;
				if (q + 1 < lineEnd)
					q += 2;
				emit(p, q, hlComment());
				p = q;
				continue;
			}

			if (*p == '#' && onlySpacesBefore(lineBegin, p))
			{
				emit(p, lineEnd, hlPreproc());
				break;
			}

			if (*p == '"')
			{
				const char *q = p + 1;
				while (q < lineEnd)
				{
					if (*q == '\\' && q + 1 < lineEnd)
					{
						q += 2;
						continue;
					}
					if (*q == '"')
					{
						++q;
						break;
					}
					++q;
				}
				emit(p, q, hlString());
				p = q;
				continue;
			}

			if (*p == '\'')
			{
				const char *q = p + 1;
				while (q < lineEnd)
				{
					if (*q == '\\' && q + 1 < lineEnd)
					{
						q += 2;
						continue;
					}
					if (*q == '\'')
					{
						++q;
						break;
					}
					++q;
				}
				emit(p, q, hlString());
				p = q;
				continue;
			}

			if (std::isdigit(static_cast<unsigned char>(*p)) || (*p == '.' && p + 1 < lineEnd && std::isdigit(static_cast<unsigned char>(p[1]))))
			{
				const char *q = p;
				if (*q == '0' && q + 1 < lineEnd && (q[1] == 'x' || q[1] == 'X'))
				{
					q += 2;
					while (q < lineEnd && std::isxdigit(static_cast<unsigned char>(*q)))
						++q;
				}
				else
				{
					while (q < lineEnd && std::isdigit(static_cast<unsigned char>(*q)))
						++q;
					if (q < lineEnd && *q == '.')
					{
						++q;
						while (q < lineEnd && std::isdigit(static_cast<unsigned char>(*q)))
							++q;
					}
					if (q < lineEnd && (*q == 'e' || *q == 'E'))
					{
						++q;
						if (q < lineEnd && (*q == '+' || *q == '-'))
							++q;
						while (q < lineEnd && std::isdigit(static_cast<unsigned char>(*q)))
							++q;
					}
					if (q < lineEnd && (*q == 'f' || *q == 'F' || *q == 'l' || *q == 'L' || *q == 'u' || *q == 'U'))
						++q;
				}
				emit(p, q, hlNumber());
				p = q;
				continue;
			}

			if (asciiIdentStart(*p))
			{
				const char *q = p + 1;
				while (q < lineEnd && asciiIdentCont(*q))
					++q;
				const std::string_view id(p, static_cast<size_t>(q - p));
				emit(p, q, isCppKeyword(id) ? hlKeyword() : defaultTextCol);
				p = q;
				continue;
			}

			emit(p, p + 1, defaultTextCol);
			++p;
		}
	}
}
