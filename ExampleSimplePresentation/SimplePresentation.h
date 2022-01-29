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
	bbe::List<bbe::Line2> lines; // Additional lines to be drawn, e.g. for cross out text.
	bbe::List<bbe::Image> images;
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
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) = 0;
	virtual void animateTokens() = 0;
	virtual bool hasFinalBrightState() = 0;
	virtual bool isTextBased() { return true; }
};

class CppTokenizer : public Tokenizer
{
public:
	const bbe::List<char> singleSignTokens = { '{', '}', '(', ')', '[', ']', ';', '*', '<', '>', '=', '.', '&', '+' };

	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};

class LineTokenizer : public Tokenizer
{
public:
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};

class BrotDownTokenizer : public Tokenizer
{
public:
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};

class AsmTokenizer : public Tokenizer
{
public:
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override;
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override;
	virtual void animateTokens() override;
	virtual bool hasFinalBrightState() override;
};

class PngTokenizer : public Tokenizer
{
public:
	virtual void tokenize(const bbe::String& text, const bbe::Font& font) override { };
	virtual void determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues) override { };
	virtual void animateTokens() override {} ;
	virtual bool hasFinalBrightState() override { return false; };
	virtual bool isTextBased() override { return false; }
	
	void loadImage(const bbe::String& path);
};

enum class BrightStateOverride
{
	NO_OVERRIDE,
	OVERRIDE_ON,
	OVERRIDE_OFF,
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
	bbe::List<bbe::String> additionalValues;
	bbe::Rectangle textAabb;
	bbe::String text;
	std::shared_ptr<Tokenizer> tokenizer;
	float scrollValue = 0;
	bool scrollingAllowed = false;
	uint32_t forcedFontSize = 0;
	bool complete = false;
	static constexpr int DEFAULT_BORDERWIDTH = 10;
	bbe::Rectangle screenPosition = bbe::Rectangle(DEFAULT_BORDERWIDTH, DEFAULT_BORDERWIDTH, 1280 - 2 * DEFAULT_BORDERWIDTH, 720 - 2 * DEFAULT_BORDERWIDTH);
	bbe::List<Slide> childSlides;
	BrightStateOverride brightStateOverride = BrightStateOverride::NO_OVERRIDE;

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
	void addValue(const bbe::String& value);
	bool isFirstEntry() const;
	bool isLastEntry() const;
	int32_t getAmountOfEntries() const;

	void forceFontSize(uint32_t size);
	void setComplete(bool complete);

	void compile();

	bbe::String getPowerPointContent(int32_t index);

	void addText(const char* txt);
	void setScreenPosition(const bbe::Rectangle& rect);

	bool hasFinalBrightState() const;

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
	int borderWidth = Slide::DEFAULT_BORDERWIDTH;

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
