#pragma once
#include "BBE/BrotBoxEngine.h"
#include <array>
#include <memory>

class SimplePresentation
{
public:
	enum class PresentationControl
	{
		none,
		next,
		previous,
	};

	enum class SimplePresentationType
	{
		cpp,
		lines,
	};
private:
	uint32_t currentEntry = 0;
	uint32_t amountOfEntries = 0;
	bool dirty = true;
	std::array<bbe::Font, 64> fonts;
	bbe::Font* selectedFont = nullptr;
	bbe::List<bbe::String> additionalTypes;
	bbe::Rectangle textAabb;

	struct Char
	{
		int32_t c = '\0';
		bbe::Vector2 pos;
	};
	enum class TokenType
	{
		unknown,
		comment,
		string,
		keyword,
		number,
		punctuation,
		function,
		namespace_,
		type,
		value,
		preprocessor,
		include_path,
		text,
	};
	struct Token
	{
		bbe::List<Char> chars;
		bbe::Rectangle aabb;
		bbe::String text;
		TokenType type = TokenType::unknown;
		int32_t showIndex = -1;

		void submit(bbe::List<Token>& tokens);
	};

	bbe::String text;

	static bbe::Color tokenTypeToColor(TokenType type);

	class Tokenizer
	{
	public:
		bbe::List<Token> tokens;

		virtual void tokenize(const bbe::String& text, const bbe::Font& font) = 0;
		virtual void determineTokenTypes(const bbe::List<bbe::String> &additionalTypes) = 0;
		virtual void animateTokens() = 0;
		virtual bool hasFinalBrightState() = 0;
	};

	class CppTokenizer : public Tokenizer
	{
	public:
		const bbe::List<char> singleSignTokens = { '{', '}', '(', ')', '[', ']', ';', '*', '<', '>', '=', '.', '&', '+' };

		virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
		virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes) override;
		virtual void animateTokens() override;
		virtual bool hasFinalBrightState() override;
	};

	class LineTokenizer : public Tokenizer
	{
	public:
		virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
		virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes) override;
		virtual void animateTokens() override;
		virtual bool hasFinalBrightState() override;
	};

	std::unique_ptr<Tokenizer> tokenizer;
	float scrollValue = 0;
	bool scrollingAllowed = false;

public:
	SimplePresentation(SimplePresentationType simplePresentationType);

	void addText(const char* txt);

	void update(PresentationControl pc, float scrollValue);
	void draw(bbe::PrimitiveBrush2D& brush);
	void addType(const bbe::String& type);

private:
	void next();
	bool hasNext() const;
	void prev();
	bool hasPrev() const;

	void compile();
	bbe::Font& getFont();


	SimplePresentation(const SimplePresentation&)            = delete;
	SimplePresentation(SimplePresentation&&)                 = delete;
	SimplePresentation& operator=(const SimplePresentation&) = delete;
	SimplePresentation& operator=(SimplePresentation&&)      = delete;
};
