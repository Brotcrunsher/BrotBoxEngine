#include "CppTokenizer.h"
#include "BBE/Vector2.h"

void CppTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);

	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
	}

	Token currentToken;

	bool lineComment = false;
	bool isString = false;
	int64_t multiComment = 0;
	int64_t forceAdd = 0;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		forceAdd--;
		if (text[i] == '\n') lineComment = false;
		if (text.isTextAtLocation("//", i)) lineComment = true;
		if (text.isTextAtLocation("/*", i)) multiComment++;
		if (text.isTextAtLocation("*/", i))
		{
			multiComment--;
			if (multiComment == 0)
			{
				forceAdd = 2;
			}
		}
		if (text.isTextAtLocation("->", i))
		{
			currentToken.submit(tokens);
			forceAdd = 2;
		}
		if (i >= 2 && text.isTextAtLocation("->", i - 2))
		{
			currentToken.submit(tokens);
		}
		if (text[i] == '"') isString = !isString;

		if (!isString)
		{
			if (singleSignTokens.contains(text[i]) && !lineComment && multiComment == 0 && forceAdd <= 0)
			{
				currentToken.submit(tokens);
			}
			if ((text.isTextAtLocation("::", i) || (i >= 2 && text.isTextAtLocation("::", i - 2))) && !lineComment && multiComment == 0)
			{
				currentToken.submit(tokens);
			}
			if (text.isTextAtLocation(",", i))
			{
				currentToken.submit(tokens);
			}
		}
		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
		{
			Char c;
			c.c = text.getCodepoint(i);
			c.pos = renderPositions[i];
			c.powerPointPos = powerPointPositions[i];

			currentToken.chars.add(c);
			currentToken.text += text[i];
		}
		else
		{
			if (!lineComment && multiComment == 0 && forceAdd <= 0 && !isString) currentToken.submit(tokens);
		}
		if (singleSignTokens.contains(text[i]) && !lineComment && multiComment == 0 && forceAdd <= 0 && !isString)
		{
			currentToken.submit(tokens);
		}
	}

	if (currentToken.chars.getLength() > 0)
	{
		tokens.add(currentToken);
	}
}

void CppTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues)
{
	const bbe::List<bbe::String> keywords = { "constexpr", "class", "public", "private", "protected", "public:", "private:", "protected:", "virtual", "override", "if", "else", "alignas", "alignof", "delete", "operator", "sizeof", "template", "typename", "struct", "return", "noexcept", "this", "void", "int", "float", "double", "co_yield", "co_return", "co_await", "true", "false", "auto", "char", "int64_t", "const", "mutable", "while", "for", "do", "noexcept", "static", "volatile", "throw", "decltype", "typeof" };
	const bbe::List<bbe::String> preprocessor = { "#include", "#define", "#ifdef", "#endif" };
	bbe::List<bbe::String> types = { "int", "char", "float", "bool", "__m128", "__m256i", "__m256d" };
	types.addList(additionalTypes);
	types.add("uint32_t");
	types.add("int32_t");
	types.add("T");
	types.add("GLfloat");
	types.add("GLint");
	types.add("GLuint");
	types.add("GL_FLOAT");
	types.add("GL_FALSE");
	types.add("GL_ARRAY_BUFFER");
	types.add("GL_ELEMENT_ARRAY_BUFFER");
	types.add("GL_COLOR_BUFFER_BIT");
	types.add("GL_TRIANGLES");
	types.add("GL_UNSIGNED_INT");
	types.add("GL_DEPTH_BUFFER_BIT");
	types.add("GL_STATIC_DRAW");
	types.add("GL_VERTEX_SHADER");
	types.add("GL_FRAGMENT_SHADER");
	types.add("NULL");
	types.add("GLEW_OK");
	bbe::List<bbe::String> values = {};
	values.addList(additionalValues);

	bool change = true;
	bool withinInclude = false;
	while (change)
	{
		change = false;
		for (size_t i = 0; i < tokens.getLength(); i++)
		{
			if (tokens[i].type != TokenType::unknown) continue;

			if (tokens[i].text.endsWith(">") || tokens[i].text.trim().endsWith("\""))
			{
				withinInclude = false;
			}

			bool didChangeThisToken = true;
			if (tokens[i].text.startsWith("//")
				|| tokens[i].text.startsWith("/*"))
			{
				tokens[i].type = TokenType::comment;
			}
			else if (preprocessor.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::preprocessor;
			}
			else if (withinInclude || (i >= 2 && (tokens[i - 1].text == "<" || tokens[i - 1].text == "\"") && tokens[i - 2].text == "#include"))
			{
				withinInclude = true;
				tokens[i].type = TokenType::include_path;
			}
			else if (tokens[i].text.startsWith("\""))
			{
				tokens[i].type = TokenType::string;
			}
			else if (keywords.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::keyword;
			}
			else if (tokens[i].text.isNumber()
				|| (tokens[i].text == "." && i > 0 && tokens[i - 1].type == TokenType::number)
				|| (tokens[i].text.endsWith("f") && i > 0 && tokens[i - 1].type == TokenType::number))
			{
				tokens[i].type = TokenType::number;
			}
			else if ((tokens[i].text == "::") || (tokens[i].text == "->") || (tokens[i].text == "%") || (tokens[i].text == "-") || (tokens[i].text == "/") || (tokens[i].text == ",") || (tokens[i].text.getLength() == 1 && singleSignTokens.contains(tokens[i].text[0])))
			{
				tokens[i].type = TokenType::punctuation;
			}
			else if (types.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::type;
			}
			else if (i != tokens.getLength() - 1 && tokens[i + 1].text == "(")
			{
				tokens[i].type = TokenType::function;
			}
			else if (i != tokens.getLength() - 1 && tokens[i + 1].text == "::")
			{
				tokens[i].type = TokenType::namespace_;
			}
			else if (tokens[i].text == ":")
			{
				tokens[i].type = TokenType::punctuation;
			}
			else if ((i > 0 && (tokens[i - 1].text == "struct"
				|| tokens[i - 1].text == "typename"
				|| tokens[i - 1].text == "class"
				|| tokens[i - 1].text == "::"))
				|| types.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::type;
				if (!types.contains(tokens[i].text)) types.add(tokens[i].text);
			}
			else if ((i > 0 && tokens[i - 1].type == TokenType::type)
				|| (i > 0 && types.contains(tokens[i - 1].text))
				|| (i > 0 && tokens[i - 1].text == "=")
				|| (i > 0 && tokens[i - 1].text == "return")
				|| (i < tokens.getLength() - 1 && tokens[i + 1].text == "=")
				|| (i < tokens.getLength() - 1 && tokens[i + 1].text == ")")
				|| (values.contains(tokens[i].text))
				|| (i > 1 && tokens[i - 2].type == TokenType::type && (tokens[i - 1].text == "*" || tokens[i - 1].text == "&")))
			{
				tokens[i].type = TokenType::value;
				if (!values.contains(tokens[i].text)) values.add(tokens[i].text);
			}
			else if (i > 0 && tokens[i - 1].text == "#ifdef")
			{
				tokens[i].type = TokenType::preprocessor;
			}
			else
			{
				didChangeThisToken = false;
			}

			if (didChangeThisToken)
			{
				change = true;
			}
		}
	}
}

void CppTokenizer::animateTokens()
{
	int32_t currentShowIndex = -1;
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		if (tokens[i].showIndex != -1)
		{
			continue;
		}

		if (i > 0 && tokens[i - 1].type == TokenType::type && tokens[i].text == "<" && (i == tokens.getLength() - 1 || tokens[i + 1].text != "<"))
		{
			int32_t depth = 0;
			for (size_t k = i; k < tokens.getLength(); k++)
			{
				tokens[k].showIndex = currentShowIndex;
				if (tokens[k].text == "<" && (k == tokens.getLength() - 1 || tokens[k + 1].text != "<"))
				{
					depth++;
				}
				else if (tokens[k].text == ">")
				{
					depth--;
					if (depth == 0)
					{
						break;
					}
				}
			}
		}
		else if (tokens[i].text == "cout" || tokens[i].text == "endl")
		{
			tokens[i].showIndex = currentShowIndex;
		}
		else if (tokens[i].type == TokenType::punctuation)
		{
			tokens[i].showIndex = currentShowIndex;

			if (tokens[i].text == "{")
			{
				int32_t depth = 0;
				for (size_t k = i; k < tokens.getLength(); k++)
				{
					if (tokens[k].text == "{")
					{
						depth++;
					}
					else if (tokens[k].text == "}")
					{
						depth--;
						if (depth == 0)
						{
							tokens[k].showIndex = currentShowIndex;
							if (k < tokens.getLength() - 1 && tokens[k + 1].text == ";")
							{
								tokens[k + 1].showIndex = currentShowIndex;
							}
							break;
						}
					}
				}
			}

			if (tokens[i].text == "(")
			{
				int32_t depth = 0;
				for (size_t k = i; k < tokens.getLength(); k++)
				{
					if (tokens[k].text == "(")
					{
						depth++;
					}
					else if (tokens[k].text == ")")
					{
						depth--;
						if (depth == 0)
						{
							tokens[k].showIndex = currentShowIndex;
							break;
						}
					}
				}
			}
		}
		else if (tokens[i].text == "for")
		{
			currentShowIndex++;
			tokens[i].showIndex = currentShowIndex;

			currentShowIndex++;

			for (size_t k = i + 1; k < tokens.getLength(); k++)
			{
				if (tokens[k].text == "(" || tokens[k].text == ")")
				{
					tokens[k].showIndex = tokens[i].showIndex;
				}
				else
				{
					tokens[k].showIndex = currentShowIndex;
				}

				if (tokens[k].text == ";")
				{
					currentShowIndex++;
				}

				if (tokens[k].text == ")")
				{
					break;
				}
			}
		}
		else if (tokens[i].type == TokenType::include_path)
		{
			tokens[i].showIndex = currentShowIndex;
		}
		else if (tokens[i].type == TokenType::number && i > 0 && tokens[i - 1].type == TokenType::number)
		{
			tokens[i].showIndex = currentShowIndex;
		}
		else
		{
			currentShowIndex++;
			tokens[i].showIndex = currentShowIndex;
		}
	}
}

bool CppTokenizer::hasFinalBrightState()
{
	return true;
}
