#pragma once

#include "Tokenizer.h"

class BrotDownTokenizer : public Tokenizer
{
private:
	bbe::String parentPath;
public:
	BrotDownTokenizer(const bbe::String& parentPath);
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};
