#include "LineTokenizer.h"

void LineTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);
	if (renderPositions.getLength() != text.getLength())
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	Token currentToken;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		if (text[i] == '\n' && (i == 0 || text[i - 1] != '\\'))
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

void LineTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues)
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].type = TokenType::text;
	}
}

void LineTokenizer::animateTokens()
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].showIndex = i;
	}
}

bool LineTokenizer::hasFinalBrightState()
{
	return false;
}
