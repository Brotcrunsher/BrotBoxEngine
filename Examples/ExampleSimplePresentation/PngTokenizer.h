#pragma once

#include "Tokenizer.h"

class PngTokenizer : public Tokenizer
{
public:
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override { };
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override { };
	virtual void animateTokens() override {};
	virtual bool hasFinalBrightState() override { return false; };
	virtual bool isTextBased() override { return false; }

	void loadImage(const bbe::String& path);
};
