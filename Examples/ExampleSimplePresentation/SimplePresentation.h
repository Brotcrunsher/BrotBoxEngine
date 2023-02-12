#pragma once
#include "BBE/BrotBoxEngine.h"
#include <array>
#include <memory>
#include "Tokenizer.h"

enum class PresentationControl
{
	none,
	next,
	previous,
	next_slide,
	previous_slide,
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
	float animationTime = 0;

public:
	Slide();
	Slide(Tokenizer* tokenizer);
	Slide(const char* path);
	~Slide();
	Slide(Slide&& other);
	Slide(const Slide&);
	Slide& operator=(Slide&&);


	void update(PresentationControl pc, float scrollValue, float timeSinceLastFrame);
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
	bool isEntryAutoNext(int32_t entry) const;


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

	void update(PresentationControl pc, float scrollValue, float timeSinceLastFrame);
	void draw(bbe::PrimitiveBrush2D& brush);
	void addType(const bbe::String& type);
	void addSlide(const char* path);
	void addSlide(const bbe::String& path);
	void addSlide(Slide &&slide);
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
