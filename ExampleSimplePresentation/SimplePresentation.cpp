#include "BBE/BrotBoxEngine.h"
#include "SimplePresentation.h"

int32_t Slide::fontsLoaded = 0;
std::array<bbe::Font, 64> Slide::fonts;

bbe::Color Slide::tokenTypeToColor(TokenType type)
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

Slide::Slide(const char* path)
{
	bbe::String ppath = path;
	bbe::String fileContent = bbe::simpleFile::readFile(ppath);
	bool typeInFile = true;

	if (fileContent.startsWith("//c++") || fileContent.startsWith("//cpp"))
	{
		tokenizer.reset(new CppTokenizer());
	}
	else if (fileContent.startsWith("//lines"))
	{
		tokenizer.reset(new LineTokenizer());
	}
	else if (fileContent.startsWith("//asm"))
	{
		tokenizer.reset(new AsmTokenizer());
	}
	else
	{
		typeInFile = false;
		if (ppath.endsWith(".cpp"))
		{
			tokenizer.reset(new CppTokenizer());
		}
		else if (ppath.endsWith(".txt"))
		{
			tokenizer.reset(new LineTokenizer());
		}
		else
		{
			throw bbe::IllegalArgumentException();
		}
	}

	addText(fileContent.getRaw() + (typeInFile ? fileContent.search("\n") : 0));

	if (!fontsLoaded)
	{
		for (size_t i = 0; i < fonts.size(); i++)
		{
			fonts[i].load("consola.ttf", i + 1);
		}
	}
	fontsLoaded++;
}

Slide::~Slide()
{
	fontsLoaded--;
	if (fontsLoaded == 0)
	{
		for (size_t i = 0; i < fonts.size(); i++)
		{
			fonts[i].destroy();
		}
	}
}

Slide::Slide(Slide&& other)
{
	currentEntry     = std::move(other.currentEntry);
	amountOfEntries  = std::move(other.amountOfEntries);
	dirty            = std::move(other.dirty);
	selectedFont     = std::move(other.selectedFont);
	additionalTypes  = std::move(other.additionalTypes);
	textAabb         = std::move(other.textAabb);
	text             = std::move(other.text);
	tokenizer        = std::move(other.tokenizer);
	scrollValue      = std::move(other.scrollValue);
	scrollingAllowed = std::move(other.scrollingAllowed);

	fontsLoaded++;
}

void Slide::addText(const char* txt)
{
	dirty = true;

	text += bbe::String(txt).replace("\t", "    ");
}

bool Slide::isFirstEntry() const
{
	return currentEntry == 0;
}

void Slide::update(PresentationControl pc, float scrollValue)
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

bool Slide::isLastEntry() const
{
	return currentEntry == amountOfEntries;
}

uint32_t Slide::getAmountOfEntries() const
{
	return amountOfEntries + (tokenizer->hasFinalBrightState() ? 1 : 0);
}

void Slide::forceFontSize(uint32_t size)
{
	for (size_t i = 0; i < fonts.size(); i++)
	{
		if (fonts[i].getFontSize() == size)
		{
			forcedFontSize = size;
			return;
		}
	}
	throw bbe::IllegalArgumentException();
}

void Slide::draw(bbe::PrimitiveBrush2D& brush, const bbe::Color& bgColor)
{
	if (dirty)
	{
		compile();
	}

	brush.setColorRGB(bgColor);
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

void Slide::addType(const bbe::String& type)
{
	additionalTypes.add(type);
}

void Slide::next()
{
	if (hasNext()) currentEntry++;
	std::cout << "Now at Entry: " << currentEntry << std::endl;
}

bool Slide::hasNext() const
{
	return currentEntry < amountOfEntries;
}

void Slide::prev()
{
	if (hasPrev()) currentEntry--;
	std::cout << "Now at Entry: " << currentEntry << std::endl;
}

bool Slide::hasPrev() const
{
	return currentEntry > 0;
}

void Slide::compile()
{
	if(!forcedFontSize) selectedFont = nullptr;
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

bbe::String Slide::getPowerPointContent(int32_t index)
{
	const bbe::String templateContent = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateContent.xml");
	const int32_t slideWidth = 12192000;
	const int32_t slideHeight = 6858000;

	bbe::String retVal = "";

	int32_t id = 1;

	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		if (tokenizer->tokens[i].showIndex > index) continue;
		for(size_t k = 0; k<tokenizer->tokens[i].chars.getLength(); k++)
		{
			id++;
			const bbe::Vector2 offset = -textAabb.getPos() - textAabb.getDim() * 0.5f + bbe::Vector2(1280, 720) * 0.5f - bbe::Vector2(0, getFont().getPixelsFromLineToLine());
			const bbe::Vector2 screenSpace = tokenizer->tokens[i].chars[k].powerPointPos + offset;
			const bbe::Vector2 normSpace = screenSpace / bbe::Vector2(1280, 720);
			const bbe::Vector2 powerPointSpace = bbe::Vector2(normSpace.x * slideWidth, normSpace.y * slideHeight);

			// The multiplication of the font size of stb to power point format.
			// I don't really have a clue why it is exactly this number, but I
			// did some pixel measurements and it seems to be correct.
			constexpr uint32_t stbToPowerPointFontSize = 79;

			const bbe::String text = bbe::String::fromCodePoint(tokenizer->tokens[i].chars[k].c)
				.replace("&", "&amp;")
				.replace("<", "&lt;")
				.replace(">", "&gt;");

			bbe::Color color = tokenTypeToColor(tokenizer->tokens[i].type);
			if (!(tokenizer->tokens[i].showIndex == index || (index == amountOfEntries && tokenizer->hasFinalBrightState())))
			{
				color *= 0.8;
			}

			retVal += templateContent
				.replace("%%%XPOS%%%", bbe::String((int32_t)(powerPointSpace.x)))
				.replace("%%%YPOS%%%", bbe::String((int32_t)(powerPointSpace.y)))
				.replace("%%%TEXT%%%", text)
				.replace("%%%ID%%%", bbe::String(id))
				.replace("%%%SIZE%%%", bbe::String(getFont().getFontSize() * stbToPowerPointFontSize))
				.replace("%%%COLOR%%%", color.toHex());
		}
	}

	return retVal;
}

bbe::Font& Slide::getFont()
{
	if (selectedFont == nullptr)
	{
		selectedFont = &fonts[0];
		scrollingAllowed = true;
		for (size_t i = 0; i < fonts.size(); i++)
		{
			if (forcedFontSize != 0)
			{
				if (fonts[i].getFontSize() != forcedFontSize)
				{
					continue;
				}
			}
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

			if ((textAabb.getHeight() < 720 - 10 && textAabb.getWidth() < 1280 - 10) || forcedFontSize != 0)
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

void Slide::Token::submit(bbe::List<Token>& tokens)
{
	if (chars.getLength() > 0)
	{
		tokens.add(*this);
	}
	*this = Token();
}

void Slide::CppTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
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

void Slide::CppTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes)
{
	const bbe::List<bbe::String> keywords = { "constexpr", "class", "public", "private", "protected", "virtual", "override", "if", "else", "alignas", "alignof", "delete", "operator", "sizeof", "template", "typename", "struct", "return", "noexcept", "this", "void", "int", "float", "co_yield", "co_return", "co_await", "true", "false", "auto", "char", "const", "mutable", "while", "for", "do", "noexcept"};
	const bbe::List<bbe::String> preprocessor = { "#include", "#define", "#ifdef", "#endif" };
	bbe::List<bbe::String> types = { "int", "char", "float" };
	types.addList(additionalTypes);
	types.add("uint32_t");
	types.add("int32_t");
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

void Slide::CppTokenizer::animateTokens()
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
		else
		{
			currentShowIndex++;
			tokens[i].showIndex = currentShowIndex;
		}
	}
}

bool Slide::CppTokenizer::hasFinalBrightState()
{
	return true;
}

void Slide::LineTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::List<bbe::Vector2> renderPositions     = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);
	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
	}

	Token currentToken;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		if (text[i] == '\n' && (i == 0 || text[i-1] != '\\'))
		{
			currentToken.submit(tokens);
		}

		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r' && !(i != renderPositions.getLength() - 1 && text[i] == '\\' && text[i + 1] == '\n'))
		{
			Char c;
			c.c = text.getCodepoint(i);
			c.pos = renderPositions[i];
			c.powerPointPos = powerPointPositions[i];

			currentToken.chars.add(c);
			currentToken.text += bbe::String::fromCodePoint(c.c);
		}
	}
	currentToken.submit(tokens);
}

void Slide::LineTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes)
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].type = TokenType::text;
	}
}

void Slide::LineTokenizer::animateTokens()
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].showIndex = i;
	}
}

bool Slide::LineTokenizer::hasFinalBrightState()
{
	return false;
}

void Slide::AsmTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::List<bbe::Vector2> renderPositions     = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);
	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
	}

	Token currentToken;
	bool beginningOfLine = true;
	bool fullLineMode = true;
	bool firstTokenOfLine = true;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		if (text[i] == '\n')
		{
			if (fullLineMode) currentToken.type = TokenType::comment;
			else if (firstTokenOfLine) currentToken.type = TokenType::function;
			else if(currentToken.type == TokenType::unknown) currentToken.type = TokenType::value;
			currentToken.submit(tokens);
		}
		if (i == 0 || text[i - 1] == '\n')
		{
			beginningOfLine = true;
			fullLineMode = true;
			firstTokenOfLine = true;
		}
		else
		{
			beginningOfLine = false;
		}

		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
		{
			Char c;
			c.c = text.getCodepoint(i);
			c.pos = renderPositions[i];
			c.powerPointPos = powerPointPositions[i];

			currentToken.chars.add(c);
			currentToken.text += bbe::String::fromCodePoint(c.c);
		}
		else if (beginningOfLine)
		{
			fullLineMode = false;
		}
		else if(!fullLineMode)
		{
			if (currentToken.chars.getLength() > 0)
			{
				if (firstTokenOfLine)
				{
					currentToken.type = TokenType::function;
					firstTokenOfLine = false;
				}
				else if (currentToken.type == TokenType::unknown)
				{
					currentToken.type = TokenType::value;
				}
			}
			currentToken.submit(tokens);
		}
	}
	currentToken.type = TokenType::value;
	currentToken.submit(tokens);
}

void Slide::AsmTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes)
{
}

void Slide::AsmTokenizer::animateTokens()
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].showIndex = 0;
	}
}

bool Slide::AsmTokenizer::hasFinalBrightState()
{
	return true;
}

void SlideShow::update(PresentationControl pc, float scrollValue)
{
	if (pc == PresentationControl::none)
	{
		slides[currentSlide].update(pc, scrollValue);
	}
	else if (pc == PresentationControl::previous)
	{
		if (slides[currentSlide].isFirstEntry())
		{
			if (currentSlide > 0)
			{
				currentSlide--;
			}
		}
		else
		{
			slides[currentSlide].update(pc, scrollValue);
		}
	}
	else if (pc == PresentationControl::next)
	{
		if (slides[currentSlide].isLastEntry())
		{
			if (currentSlide < slides.getLength() - 1)
			{
				currentSlide++;
			}
		}
		else
		{
			slides[currentSlide].update(pc, scrollValue);
		}
	}
	else if (pc == PresentationControl::previous_slide)
	{
		if (currentSlide > 0)
		{
			currentSlide--;
		}
	}
	else if (pc == PresentationControl::next_slide)
	{
		if (currentSlide < slides.getLength() - 1)
		{
			currentSlide++;
		}
	}
	else
	{
		throw bbe::IllegalArgumentException();
	}
}

void SlideShow::draw(bbe::PrimitiveBrush2D& brush)
{
	slides[currentSlide].draw(brush, bgColor);
}

void SlideShow::addType(const bbe::String& type)
{
	for (size_t i = 0; i < slides.getLength(); i++)
	{
		slides[i].addType(type);
	}
}

void SlideShow::addSlide(const char* path)
{
	slides.add(Slide(path));
}

void SlideShow::addSlide(const bbe::String& path)
{
	addSlide(path.getRaw());
}

void SlideShow::addManifest(const char* inPath)
{
	const bbe::String path = inPath;
	const bbe::String parentPath = path.substring(0, path.searchLast("/"));
	const bbe::String fileContent = bbe::simpleFile::readFile(path);
	const bbe::DynamicArray<bbe::String> lines = fileContent.split("\n");

	for (size_t i = 0; i < lines.getLength(); i++)
	{
		const bbe::String line = lines[i].trim();
		const bbe::DynamicArray<bbe::String> tokens = line.split(" ");
		if (tokens.getLength() == 0) continue;

		if (tokens[0] == "SLIDE:")
		{
			addSlide(parentPath + tokens[1]);
		}
	}
}

void SlideShow::forceFontSize(uint32_t fontSize)
{
	for (size_t i = 0; i < slides.getLength(); i++)
	{
		forceFontSize(fontSize, i);
	}
}

void SlideShow::forceFontSize(uint32_t fontSize, uint32_t slide)
{
	slides[slide].forceFontSize(fontSize);
}

void SlideShow::writeAsPowerPoint(const bbe::String& path)
{
	for (size_t i = 0; i < slides.getLength(); i++)
	{
		if (slides[i].dirty)
		{
			slides[i].compile();
		}
	}

	// TODO Remove absolute paths
	const bbe::String templateOverall = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateOverall.xml");
	const bbe::String templateSlides = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateSlides.xml");
	const bbe::String templateSlideRel = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateSlideRel.xml");
	const bbe::String templateSlideIdList = "<p:sldId id=\"%%%ID%%%\" r:id=\"rId%%%ID%%%\"/>";
	const bbe::String templateRelationships = "<Relationship Id=\"rId%%%ID%%%\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide\" Target=\"slides/slide%%%ID%%%.xml\"/>";

	bbe::String slides = "";
	bbe::String slideRels = "";
	bbe::String slideIdList = "";
	bbe::String relationships = "";

	uint32_t id = 256;

	for (int i = 0; i < this->slides.getLength(); i++)
	{
		uint32_t amountOfEntries = this->slides[i].getAmountOfEntries();
		for (uint32_t entry = 0; entry < amountOfEntries; entry++)
		{
			id++;
			const bbe::String stringId = bbe::String(id);

			const bbe::String powerPointContent = this->slides[i].getPowerPointContent(entry);

			slides += templateSlides.replace("%%%ID%%%", stringId)
				.replace("%%%BG_COLOR%%%", bgColor.toHex())
				.replace("%%%CONTENT%%%", powerPointContent);
			slideRels     += templateSlideRel     .replace("%%%ID%%%", stringId);
			slideIdList   += templateSlideIdList  .replace("%%%ID%%%", stringId);
			relationships += templateRelationships.replace("%%%ID%%%", stringId);
		}
	}


	bbe::String output = templateOverall
		.replace("%%%SLIDES_TEMPLATE%%%", slides)
		.replace("%%%SLIDE_ID_LIST%%%", slideIdList)
		.replace("%%%SLIDE_RELATIONSHIPS%%%", relationships)
		.replace("%%%SLIDE_REL_TEMPLATE%%%", slideRels);

	bbe::simpleFile::writeStringToFile(path, output);
}

bbe::Rectangle SlideShow::getTextAabb(uint32_t slide) const
{
	return slides[slide].textAabb;
}

void SlideShow::setTextAabb(uint32_t slide, const bbe::Rectangle& value)
{
	slides[slide].textAabb = value;
}
