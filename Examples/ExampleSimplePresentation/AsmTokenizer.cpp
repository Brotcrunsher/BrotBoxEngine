#include "AsmTokenizer.h"

void AsmTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::List<bbe::Vector2> renderPositions     = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);
	if (renderPositions.getLength() != text.getLength())
	{
		bbe::Crash(bbe::Error::IllegalState);
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

void AsmTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues)
{
}

void AsmTokenizer::animateTokens()
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].showIndex = 0;
	}
}

bool AsmTokenizer::hasFinalBrightState()
{
	return true;
}
