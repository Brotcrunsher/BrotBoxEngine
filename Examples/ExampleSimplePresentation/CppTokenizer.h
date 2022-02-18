#pragma once

#include "Tokenizer.h"

class CppTokenizer : public Tokenizer
{
public:
	const bbe::List<char> singleSignTokens = { '{', '}', '(', ')', '[', ']', ';', '*', '<', '>', '=', '.', '&', '+' };

	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};
