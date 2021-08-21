#include "BBE/BrotBoxEngine.h"
#include "SimplePresentation.h"

bbe::Color SimplePresentation::tokenTypeToColor(TokenType type)
{
	switch (type)
	{
	case TokenType::unknown     : return bbe::Color(1  , 0 ,  1);
	case TokenType::comment     : return bbe::Color( 87.f / 255.f, 162.f / 255.f,  71.f / 255.f);
	case TokenType::string      : return bbe::Color(214.f / 255.f, 157.f / 255.f, 133.f / 255.f);
	case TokenType::keyword     : return bbe::Color( 86.f / 255.f, 156.f / 255.f, 214.f / 255.f);
	case TokenType::number      : return bbe::Color(181.f / 255.f, 206.f / 255.f, 164.f / 255.f);
	case TokenType::punctuation : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	case TokenType::function    : return bbe::Color(220.f / 255.f, 220.f / 255.f, 170.f / 255.f);
	case TokenType::namespace_  : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	case TokenType::type        : return bbe::Color( 78.f / 255.f, 201.f / 255.f, 176.f / 255.f);
	case TokenType::value       : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	case TokenType::preprocessor: return bbe::Color(155.f / 255.f, 155.f / 255.f, 155.f / 255.f);
	case TokenType::include_path: return bbe::Color(214.f / 255.f, 157.f / 255.f, 133.f / 255.f);
	case TokenType::text        : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	}

	throw bbe::IllegalArgumentException();
}

SimplePresentation::SimplePresentation(SimplePresentationType simplePresentationType)
{
	switch (simplePresentationType)
	{
	case SimplePresentationType::cpp:
		tokenizer.reset(new CppTokenizer());
		break;
	case SimplePresentationType::lines:
		tokenizer.reset(new LineTokenizer());
		break;
	default:
		throw bbe::IllegalStateException();
	}

	for (size_t i = 0; i < fonts.size(); i++)
	{
		fonts[i].load("consola.ttf", i + 20);
	}
}

void SimplePresentation::addText(const char* txt)
{
	dirty = true;

	text += bbe::String(txt).replace("\t", "    ");
}

void SimplePresentation::update(PresentationControl pc, float scrollValue)
{
	if (pc == PresentationControl::none)
	{
		// do nothing
	}
	else if (pc == PresentationControl::next)
	{
		next();
	}
	else if (pc == PresentationControl::previous)
	{
		prev();
	}
	else
	{
		throw bbe::IllegalArgumentException();
	}

	if (scrollingAllowed)
	{
		this->scrollValue += scrollValue;
	}
}

void SimplePresentation::draw(bbe::PrimitiveBrush2D& brush)
{
	if (dirty)
	{
		compile();
	}

	brush.setColorRGB(0.1, 0.1, 0.1);
	brush.fillRect(0, 0, 10000, 10000);

	brush.setColorRGB(1, 1, 1);

	bbe::Vector2 offset = -textAabb.getPos() - textAabb.getDim() * 0.5f + bbe::Vector2(1280, 720) * 0.5f;
	if (offset.y < 20) offset.y = 20;
	offset.y += scrollValue;

	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		if (tokenizer->tokens[i].showIndex > currentEntry) continue;
		const Token& token = tokenizer->tokens[i];
		//brush.setColorRGB(1, 1, 0);
		//brush.sketchRect(textAabb.offset(bbe::Vector2{0, 100}));
		if (tokenizer->tokens[i].showIndex == currentEntry || (currentEntry == amountOfEntries && tokenizer->hasFinalBrightState()))
		{
			brush.setColorRGB(tokenTypeToColor(token.type));
		}
		else
		{
			//brush.setColorRGB(0.3, 0.3, 0.3);
			brush.setColorRGB(tokenTypeToColor(token.type) * 0.8f);
		}
		for (size_t k = 0; k < token.chars.getLength(); k++)
		{
			brush.fillChar(token.chars[k].pos + offset, token.chars[k].c, getFont());
		}
	}
}

void SimplePresentation::addType(const bbe::String& type)
{
	additionalTypes.add(type);
}

void SimplePresentation::next()
{
	if (hasNext()) currentEntry++;
	std::cout << "Now at Entry: " << currentEntry << std::endl;
}

bool SimplePresentation::hasNext() const
{
	return currentEntry < amountOfEntries;
}

void SimplePresentation::prev()
{
	if (hasPrev()) currentEntry--;
	std::cout << "Now at Entry: " << currentEntry << std::endl;
}

bool SimplePresentation::hasPrev() const
{
	return currentEntry > 0;
}

void SimplePresentation::compile()
{
	selectedFont = nullptr;
	tokenizer->tokenize(text, getFont());

	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		for (size_t k = 0; k < tokenizer->tokens[i].chars.getLength(); k++)
		{
			const bbe::Rectangle currentAabb = bbe::Rectangle(tokenizer->tokens[i].chars[k].pos, getFont().getDimensions(tokenizer->tokens[i].chars[k].c));
			if (k == 0)
			{
				tokenizer->tokens[i].aabb = currentAabb;
			}
			else
			{
				tokenizer->tokens[i].aabb = tokenizer->tokens[i].aabb.combine(currentAabb);
			}
		}
	}
	tokenizer->determineTokenTypes(additionalTypes);
	tokenizer->animateTokens();


	int32_t highestShowIndex = 0;
	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		if (tokenizer->tokens[i].showIndex > highestShowIndex) highestShowIndex = tokenizer->tokens[i].showIndex;
	}
	amountOfEntries = highestShowIndex + (tokenizer->hasFinalBrightState() ? 1 : 0);

	dirty = false;
}

bbe::Font& SimplePresentation::getFont()
{
	if (selectedFont == nullptr)
	{
		selectedFont = &fonts[0];
		scrollingAllowed = true;
		for (size_t i = 0; i < fonts.size(); i++)
		{
			bbe::List<bbe::Vector2> renderPositions = fonts[i].getRenderPositions(bbe::Vector2(0, 0), text);
			bbe::Rectangle textAabb = bbe::Rectangle(renderPositions[0], getFont().getDimensions(text.getCodepoint(0)));
			for (size_t k = 1; k < renderPositions.getLength(); k++)
			{
				textAabb = textAabb.combine(bbe::Rectangle(renderPositions[k], getFont().getDimensions(text.getCodepoint(k))));
			}

			if (i == 0)
			{
				// Make sure we have an Aabb, even if it is large.
				this->textAabb = textAabb;
			}
			else if (i == 1)
			{
				scrollingAllowed = false;
			}

			if (textAabb.getHeight() < 720 - 10 && textAabb.getWidth() < 1280 - 10)
			{
				selectedFont = &fonts[i];
				this->textAabb = textAabb;
			}

			if (textAabb.getHeight() > 720 - 50 || textAabb.getWidth() > 1280 - 50) break;
		}

		selectedFont->setFixedWidth(selectedFont->getAdvanceWidth(' '));
	}

	return *selectedFont;
}

void SimplePresentation::Token::submit(bbe::List<Token>& tokens)
{
	if (chars.getLength() > 0)
	{
		tokens.add(*this);
		*this = Token();
	}
}

void SimplePresentation::CppTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(bbe::Vector2(0, 0), text);
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

		if (singleSignTokens.contains(text[i]) && !lineComment && multiComment == 0 && forceAdd <= 0 && !isString)
		{
			currentToken.submit(tokens);
		}
		if ((text.isTextAtLocation("::", i) || (i >= 2 && text.isTextAtLocation("::", i - 2))) && !lineComment && multiComment == 0)
		{
			currentToken.submit(tokens);
		}
		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
		{
			Char c;
			c.c = text.getCodepoint(i);
			c.pos = renderPositions[i];

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

void SimplePresentation::CppTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes)
{
	const bbe::List<bbe::String> keywords = { "constexpr", "class", "public", "private", "protected", "virtual", "override", "if", "else", "alignas", "alignof", "delete", "operator", "sizeof", "template", "typename", "struct", "return", "noexcept", "this", "void", "int", "float", "co_yield", "true", "false", "auto", "char", "const", "mutable", "while", "for", "do" };
	const bbe::List<bbe::String> preprocessor = { "#include", "#define", "#ifdef", "#endif" };
	bbe::List<bbe::String> types = { "int", "char", "float" };
	types.addList(additionalTypes);
	bbe::List<bbe::String> values = {};

	bool change = true;
	while (change)
	{
		change = false;
		for (size_t i = 0; i < tokens.getLength(); i++)
		{
			if (tokens[i].type != TokenType::unknown) continue;

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
			else if (i >= 2 && (tokens[i - 1].text == "<" || tokens[i - 1].text == "\"") && tokens[i - 2].text == "#include")
			{
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
			else if (tokens[i].text.isNumber())
			{
				tokens[i].type = TokenType::number;
			}
			else if ((tokens[i].text == "::") || (tokens[i].text == "->") || (tokens[i].text.getLength() == 1 && singleSignTokens.contains(tokens[i].text[0])))
			{
				tokens[i].type = TokenType::punctuation;
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
				|| (i < tokens.getLength() - 1 && tokens[i + 1].text == "=")
				|| (i < tokens.getLength() - 1 && tokens[i + 1].text == ")")
				|| (values.contains(tokens[i].text)))
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

void SimplePresentation::CppTokenizer::animateTokens()
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
		else
		{
			currentShowIndex++;
			tokens[i].showIndex = currentShowIndex;
		}
	}
}

bool SimplePresentation::CppTokenizer::hasFinalBrightState()
{
	return true;
}

void SimplePresentation::LineTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(bbe::Vector2(0, 0), text);
	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
	}

	Token currentToken;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		if (text[i] == '\n')
		{
			currentToken.submit(tokens);
		}

		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
		{
			Char c;
			c.c = text.getCodepoint(i);
			c.pos = renderPositions[i];

			currentToken.chars.add(c);
			currentToken.text += text[i];
		}
	}
	currentToken.submit(tokens);
}

void SimplePresentation::LineTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes)
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].type = TokenType::text;
	}
}

void SimplePresentation::LineTokenizer::animateTokens()
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].showIndex = i;
	}
}

bool SimplePresentation::LineTokenizer::hasFinalBrightState()
{
	return false;
}
