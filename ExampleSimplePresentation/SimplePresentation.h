#pragma once
#include "BBE/BrotBoxEngine.h"
#include <array>
#include <memory>

enum class PresentationControl
{
	none,
	next,
	previous,
	next_slide,
	previous_slide,
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

struct Char
{
	int32_t c = '\0';
	bbe::Vector2 pos;
	bbe::Vector2 powerPointPos;
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

class Tokenizer
{
public:
	bbe::List<Token> tokens;

	virtual void tokenize(const bbe::String& text, const bbe::Font& font) = 0;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes) = 0;
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

class AsmTokenizer : public Tokenizer
{
public:
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};

class Slide
{
	friend class SlideShow;
private:
	static bbe::Color tokenTypeToColor(TokenType type);

	int32_t currentEntry = 0;
	int32_t amountOfEntries = 0;
	bool dirty = true;
	bbe::Font* selectedFont = nullptr;
	bbe::List<bbe::String> additionalTypes;
	bbe::Rectangle textAabb;
	bbe::String text;
	std::shared_ptr<Tokenizer> tokenizer;
	float scrollValue = 0;
	bool scrollingAllowed = false;
	uint32_t forcedFontSize = 0;
	bool complete = false;
	bbe::Rectangle screenPosition = bbe::Rectangle(0, 0, 1280, 720);
	bbe::List<Slide> childSlides;

public:
	Slide();
	Slide(Tokenizer* tokenizer);
	Slide(const char* path);
	~Slide();
	Slide(Slide&& other);
	Slide(const Slide&);
	Slide& operator=(Slide&&);


	void update(PresentationControl pc, float scrollValue);
	void draw(bbe::PrimitiveBrush2D& brush, const bbe::Color &bgColor);
	void addType(const bbe::String& type);
	bool isFirstEntry() const;
	bool isLastEntry() const;
	int32_t getAmountOfEntries() const;

	void forceFontSize(uint32_t size);
	void setComplete(bool complete);

	void compile();

	bbe::String getPowerPointContent(int32_t index);

	void addText(const char* txt);
	void setScreenPosition(const bbe::Rectangle& rect);

private:
	void loadFonts();
	void next();
	bool hasNext() const;
	void prev();
	bool hasPrev() const;
	bbe::Font& getFont();
	void moveFrom(Slide&& other);
	void copyFrom(const Slide& other);


	Slide& operator=(const Slide&) = delete;
};

class SlideShow
{
public:
	bbe::List<Slide> slides;
	uint32_t currentSlide = 0;
	static constexpr bbe::Color bgColor = bbe::Color(0.1f, 0.1f, 0.1f);

public:
	SlideShow() = default;

	void update(PresentationControl pc, float scrollValue);
	void draw(bbe::PrimitiveBrush2D& brush);
	void addType(const bbe::String& type);
	void addSlide(const char* path);
	void addSlide(const bbe::String& path);
	void addSlide(Slide &slide);
	void addManifest(const char* path);

	void forceFontSize(uint32_t fontSize);
	void forceFontSize(uint32_t fontSize, uint32_t slide);

	void writeAsPowerPoint(const bbe::String& path);

	bbe::Rectangle getTextAabb(uint32_t slide) const;
	void setTextAabb(uint32_t slide, const bbe::Rectangle& value);

	Slide& getLastSlide();
	Slide& getSecondToLastSlide();

private:
	SlideShow(const SlideShow&)            = delete;
	SlideShow(SlideShow&&)                 = delete;
	SlideShow& operator=(const SlideShow&) = delete;
	SlideShow& operator=(SlideShow&&)      = delete;


	//TODO ugly helper.
	void addSlidesToList(Slide* slide, bbe::List<Slide*>& list);

};
