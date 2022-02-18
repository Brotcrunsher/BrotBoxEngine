#pragma once

#include "BBE/List.h"
#include "BBE/String.h"
#include "BBE/Font.h"
#include "BBE/Line2.h"

constexpr float textMargin = 10;

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

enum class RenderType
{
	UNKNOWN,
	BOX,
	CIRCLE,
	TEXT,
	LINE,
	ARROW,
	IMAGE,
};

enum class StartAnimation
{
	NONE,
	ZOOM_IN,
};

enum class MoveAnimationType
{
	LINEAR,
	BEZIER,
	BEZIER2C,
};

struct MoveAnimation
{
	MoveAnimation() = default;
	MoveAnimation(float targetX, float targetY);
	MoveAnimation(float targetX, float targetY, float controlX, float controlY);
	MoveAnimation(float targetX, float targetY, float controlX, float controlY, float controlX2, float controlY2);

	MoveAnimationType mat;

	float targetX;
	float targetY;

	float controlX;
	float controlY;

	float controlX2;
	float controlY2;

	bbe::Vector2 animate(float startX, float startY, float t) const;
};

struct RenderObject
{
	RenderObject(const bbe::String& name, RenderType rt, StartAnimation startAnim, float x, float y, float width, float height, float outlineWidth, const bbe::String& text, const bbe::Font* font);

	bbe::String name;
	RenderType rt;
	float x;
	float y;
	union {
		float width;
		float x2;
	};
	union {
		float height;
		float y2;
	};
	float outlineWidth;
	bbe::String text;
	const bbe::Font* font;
	bbe::Rectangle textBoundingBox;
	std::shared_ptr<bbe::Image> image;

	StartAnimation startAnim;
	bbe::List<MoveAnimation> animations;

	bool showText = true;
	static constexpr bbe::Color defaultTextColor = bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	bbe::Color fillColor = bbe::Color(0, 0, 0, 1);
	bbe::Color outlineColor = bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	bbe::Color textColor = defaultTextColor;

	bbe::Vector2 getPos(float t) const;
	bbe::Vector2 getDim(float t) const;
	void exhaustAnimations();

	bool hasAnyAnimation() const;
};

struct Token
{
	bbe::List<Char> chars;
	bbe::List<bbe::Line2> lines; // Additional lines to be drawn, e.g. for cross out text.
	bbe::List<RenderObject> renderObjects;
	bbe::List<bbe::Image> images;
	bbe::Rectangle aabb;
	bbe::String text;
	TokenType type = TokenType::unknown;
	int32_t showIndex = -1;
	bool autoNext = false;
	float animationMultiplier = 1.f;

	void submit(bbe::List<Token>& tokens);
	bbe::List<size_t> getRenderObjectIndices(const bbe::List<bbe::String>& names) const;
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
