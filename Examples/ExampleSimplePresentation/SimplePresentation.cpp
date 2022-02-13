#include "BBE/BrotBoxEngine.h"
#include "SimplePresentation.h"

static int32_t fontsLoaded = 0;
static bbe::List<bbe::Font> fonts;
constexpr float textMargin = 10;

bbe::Color Slide::tokenTypeToColor(TokenType type)
{
	switch (type)
	{
	case TokenType::unknown     : return bbe::Color(1  , 0 ,  1);
	case TokenType::comment     : return bbe::Color( 87.f / 255.f, 162.f / 255.f,  71.f / 255.f);
	case TokenType::string      : return bbe::Color(214.f / 255.f, 157.f / 255.f, 133.f / 255.f);
	case TokenType::keyword     : return bbe::Color( 86.f / 255.f, 156.f / 255.f, 214.f / 255.f);
	case TokenType::number      : return bbe::Color(181.f / 255.f, 206.f / 255.f, 164.f / 255.f);
	case TokenType::punctuation : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	case TokenType::function    : return bbe::Color(220.f / 255.f, 220.f / 255.f, 170.f / 255.f);
	case TokenType::namespace_  : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	case TokenType::type        : return bbe::Color( 78.f / 255.f, 201.f / 255.f, 176.f / 255.f);
	case TokenType::value       : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	case TokenType::preprocessor: return bbe::Color(155.f / 255.f, 155.f / 255.f, 155.f / 255.f);
	case TokenType::include_path: return bbe::Color(214.f / 255.f, 157.f / 255.f, 133.f / 255.f);
	case TokenType::text        : return bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
	}

	throw bbe::IllegalArgumentException();
}

MoveAnimation::MoveAnimation(float targetX, float targetY)
	: targetX(targetX), targetY(targetY), mat(MoveAnimationType::LINEAR)
{
}

MoveAnimation::MoveAnimation(float targetX, float targetY, float controlX, float controlY)
	: targetX(targetX), targetY(targetY), controlX(controlX), controlY(controlY), mat(MoveAnimationType::BEZIER)
{
}

MoveAnimation::MoveAnimation(float targetX, float targetY, float controlX, float controlY, float controlX2, float controlY2)
	: targetX(targetX), targetY(targetY), controlX(controlX), controlY(controlY), controlX2(controlX2), controlY2(controlY2), mat(MoveAnimationType::BEZIER2C)
{
}

bbe::Vector2 MoveAnimation::animate(float startX, float startY, float t) const
{
	if (t <= 0) return bbe::Vector2(startX, startY);
	if (t >= 1) return bbe::Vector2(targetX, targetY);

	if (mat == MoveAnimationType::LINEAR)
		return bbe::Math::interpolateLinear(bbe::Vector2{ startX, startY }, bbe::Vector2{ targetX, targetY }, t);
	else if (mat == MoveAnimationType::BEZIER)
		return bbe::Math::interpolateBezier(bbe::Vector2{ startX, startY }, bbe::Vector2{ targetX, targetY }, t, bbe::Vector2{ controlX, controlY });
	else if (mat == MoveAnimationType::BEZIER2C)
		return bbe::Math::interpolateBezier(bbe::Vector2{ startX, startY }, bbe::Vector2{ targetX, targetY }, t, { bbe::Vector2{ controlX, controlY }, bbe::Vector2{ controlX2, controlY2 } });
	else
		throw bbe::IllegalStateException();
}

RenderObject::RenderObject(const bbe::String& name, RenderType rt, StartAnimation startAnim, float x, float y, float width, float height, float outlineWidth, const bbe::String& text, const bbe::Font* font)
	: name(name), rt(rt), startAnim(startAnim), x(x), y(y), width(width), height(height), outlineWidth(outlineWidth), text(text), font(font)
{
}

bbe::Vector2 RenderObject::getPos(float t) const
{
	t = bbe::Math::clamp01(t);
	float startAnimOffsetX = 0;
	float startAnimOffsetY = 0;

	if (startAnim == StartAnimation::NONE)
	{
		// Do nothing
	}
	else if (startAnim == StartAnimation::ZOOM_IN)
	{
		startAnimOffsetX += width  * 0.5 * (1.f - t);
		startAnimOffsetY += height * 0.5 * (1.f - t);
		if (rt == RenderType::LINE || rt == RenderType::ARROW)
		{
			// TODO Currently does not support StartAnim + moveAnim
			const bbe::Vector2 mid((x + x2) / 2, (y + y2) / 2);
			const bbe::Vector2 target(x, y);
			const bbe::Vector2 toVec = target - mid;
			return mid + toVec * t;
		}
	}
	else
	{
		throw bbe::IllegalArgumentException();
	}

	if (animations.getLength() == 0) return bbe::Vector2(x + startAnimOffsetX, y + startAnimOffsetY);
	if (t >= 1) return bbe::Vector2(animations.last().targetX, animations.last().targetY);

	t *= animations.getLength();
	const int32_t index = (int32_t)t;
	const float percentage = t - index;

	float prevX = 0;
	float prevY = 0;
	if (index > 0)
	{
		prevX = animations[index - 1].targetX + startAnimOffsetX;
		prevY = animations[index - 1].targetY + startAnimOffsetY;
	}
	else
	{
		prevX = x;
		prevY = y;
	}

	return animations[index].animate(prevX, prevY, percentage);
}

bbe::Vector2 RenderObject::getDim(float t) const
{
	t = bbe::Math::clamp01(t);
	bbe::Vector2 offset;
	if (rt == RenderType::LINE || rt == RenderType::ARROW)
	{
		offset = getPos(t) - getPos(0);
	}

	if (startAnim == StartAnimation::NONE)
	{
		return bbe::Vector2(width, height) + offset;
	}
	else if (startAnim == StartAnimation::ZOOM_IN)
	{
		if (rt == RenderType::LINE || rt == RenderType::ARROW)
		{
			const bbe::Vector2 mid((x + x2) / 2, (y + y2) / 2);
			const bbe::Vector2 target(x2, y2);
			const bbe::Vector2 toVec = target - mid;
			return mid + toVec * t;
		}
		else
		{
			return bbe::Vector2(width * t, height * t) + offset;
		}
	}
	throw bbe::IllegalStateException();
}

void RenderObject::exhaustAnimations()
{
	if (animations.getLength() > 0)
	{
		const bbe::Vector2 pos = getPos(1);
		const bbe::Vector2 dim = getDim(1);
		x = pos.x;
		y = pos.y;

		width = dim.x;
		height = dim.y;



		animations.clear();
	}
}

bool RenderObject::hasAnyAnimation() const
{
	return animations.getLength() > 0 || startAnim != StartAnimation::NONE;
}

Slide::Slide()
{
}

Slide::Slide(Tokenizer* tokenizer)
{
	this->tokenizer.reset(tokenizer);

	loadFonts();
}

Slide::Slide(const char* path)
{
	bbe::String ppath = path;

	if (ppath.endsWith(".cpp"))
	{
		tokenizer.reset(new CppTokenizer());
	}
	else if (ppath.endsWith(".txt"))
	{
		tokenizer.reset(new LineTokenizer());
	}
	else if (ppath.endsWith(".bd"))
	{
		const bbe::String parentPath = ppath.substring(0, ppath.searchLast("/")) + "/";
		tokenizer.reset(new BrotDownTokenizer(parentPath));
	}
	else if (ppath.endsWith(".asm"))
	{
		tokenizer.reset(new AsmTokenizer());
	}
	else if (ppath.endsWith(".png"))
	{
		PngTokenizer* t = new PngTokenizer();
		t->loadImage(ppath);
		tokenizer.reset(t);
	}
	else
	{
		throw bbe::IllegalArgumentException();
	}

	if(tokenizer->isTextBased()) addText(bbe::simpleFile::readFile(ppath).getRaw());

	loadFonts();
}

Slide::~Slide()
{
	fontsLoaded--;
	if (fontsLoaded == 0)
	{
		for (size_t i = 0; i < fonts.getLength(); i++)
		{
			fonts[i].destroy();
		}
	}
}

Slide::Slide(Slide&& other)
{
	moveFrom(std::move(other));

	fontsLoaded++;
}

Slide::Slide(const Slide& other)
{
	copyFrom(other);

	fontsLoaded++;
}

Slide& Slide::operator=(Slide&& other)
{
	moveFrom(std::move(other));
	fontsLoaded++;

	return *this;
}

void Slide::loadFonts()
{
	if (!fontsLoaded)
	{
		for (size_t i = 0; i < 64; i++)
		{
			fonts.add(bbe::Font());
			fonts[i].load("consola.ttf", i + 1);
			fonts[i].setFixedWidth(fonts[i].getAdvanceWidth(' '));
		}
	}
	fontsLoaded++;
}

void Slide::addText(const char* txt)
{
	dirty = true;

	text += bbe::String(txt).replace("\t", "    ");
}

void Slide::setScreenPosition(const bbe::Rectangle& rect)
{
	screenPosition = rect;
}

bool Slide::hasFinalBrightState() const
{
	if (brightStateOverride == BrightStateOverride::OVERRIDE_ON)  return true;
	if (brightStateOverride == BrightStateOverride::OVERRIDE_OFF) return false;

	return tokenizer->hasFinalBrightState();
}

bool Slide::isFirstEntry() const
{
	if (currentEntry != 0) return false;

	for (size_t i = 0; i < childSlides.getLength(); i++)
	{
		if (childSlides[i].currentEntry != (childSlides[i].complete ? 0 : -1)) return false;
	}

	return true;
}

void Slide::update(PresentationControl pc, float scrollValue, float timeSinceLastFrame)
{
	if (pc == PresentationControl::none)
	{
		// do nothing
	}
	else if (pc == PresentationControl::next)
	{
		next();
	}
	else if (pc == PresentationControl::previous)
	{
		prev();
	}
	else
	{
		throw bbe::IllegalArgumentException();
	}

	if (scrollingAllowed)
	{
		this->scrollValue += scrollValue;
	}

	float animMult = 1.0f;
	for (const Token& t : tokenizer->tokens)
	{
		if (t.animationMultiplier != 1.0f)
		{
			animMult = t.animationMultiplier;
		}
	}

	animationTime += timeSinceLastFrame * animMult;

	bool hasAnyAnimation = false;
	for (const Token& t : tokenizer->tokens)
	{
		if (t.showIndex == currentEntry)
		{
			for (const RenderObject& to : t.renderObjects)
			{
				if (to.hasAnyAnimation())
				{
					hasAnyAnimation = true;
				}
			}
		}
	}

	if (animationTime > 1.0f || !hasAnyAnimation)
	{
		for (const Token& t : tokenizer->tokens)
		{
			if (t.showIndex == currentEntry && t.autoNext)
			{
				next();
				break;
			}
		}
	}
}

bool Slide::isLastEntry() const
{
	if (currentEntry != getAmountOfEntries())
	{
		return false;
	}
	for (size_t i = 0; i < childSlides.getLength(); i++)
	{
		if (!childSlides[i].isLastEntry()) return false;
	}

	return true;
}

int32_t Slide::getAmountOfEntries() const
{
	if (complete) return 0;

	return amountOfEntries + (hasFinalBrightState() ? 1 : 0);
}

void Slide::forceFontSize(uint32_t size)
{
	for (size_t i = 0; i < fonts.getLength(); i++)
	{
		if (fonts[i].getFontSize() == size)
		{
			forcedFontSize = size;
			selectedFont = nullptr;
			return;
		}
	}
	throw bbe::IllegalArgumentException();
}

void Slide::setComplete(bool complete_)
{
	currentEntry = 0;
	complete = complete_;
}

void Slide::draw(bbe::PrimitiveBrush2D& brush, const bbe::Color& bgColor)
{
	if (dirty)
	{
		compile();
	}

	brush.setColorRGB(bgColor);
	brush.fillRect(0, 0, 10000, 10000);

	brush.setColorRGB(1, 1, 1);
	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		for (bbe::Image& img : tokenizer->tokens[i].images)
		{
			brush.drawImage(0, 0, img);
		}
	}

	bbe::Vector2 offset = -textAabb.getPos() - textAabb.getDim() * 0.5f + screenPosition.getDim() * 0.5f + screenPosition.getPos();
	if (offset.y < 20) offset.y = 20;
	offset.y += scrollValue;

	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		if (tokenizer->tokens[i].showIndex > currentEntry) continue;
		const Token& token = tokenizer->tokens[i];
		//brush.setColorRGB(1, 1, 0);
		//brush.sketchRect(textAabb.offset(bbe::Vector2{0, 100}));
		if (tokenizer->tokens[i].showIndex == currentEntry || (currentEntry == getAmountOfEntries() && hasFinalBrightState()))
		{
			brush.setColorRGB(tokenTypeToColor(token.type));
		}
		else
		{
			//brush.setColorRGB(0.3, 0.3, 0.3);
			brush.setColorRGB(tokenTypeToColor(token.type) * 0.8f);
		}
		for (size_t k = 0; k < token.chars.getLength(); k++)
		{
			brush.fillChar(token.chars[k].pos + offset, token.chars[k].c, getFont());
		}

		brush.setColorRGB(tokenTypeToColor(TokenType::text));
		for (const bbe::Line2& line : tokenizer->tokens[i].lines)
		{
			brush.fillLine(line + offset, 3);
		}
		if (tokenizer->tokens[i].showIndex == currentEntry)
		{
			for (const RenderObject& ro : tokenizer->tokens[i].renderObjects)
			{
				brush.setColorRGB(ro.outlineWidth > 0 ? ro.outlineColor : ro.fillColor);
				const bbe::Vector2 pos = ro.getPos(animationTime);
				const bbe::Vector2 dim = ro.getDim(animationTime);
				if (ro.rt == RenderType::BOX)
					brush.fillRect  (pos.x, pos.y, dim.x, dim.y);
				else if (ro.rt == RenderType::CIRCLE)
					brush.fillCircle(pos.x, pos.y, dim.x, dim.y);
				else if (ro.rt == RenderType::TEXT)
					{ /*Do nothing*/ }
				else if (ro.rt == RenderType::LINE || ro.rt == RenderType::ARROW)
				{
					brush.setColorRGB(ro.outlineColor);
					if(ro.rt == RenderType::LINE)
						brush.fillLine(pos.x, pos.y, dim.x, dim.y, ro.outlineWidth);
					else if(ro.rt == RenderType::ARROW)
						brush.fillArrow(pos.x, pos.y, dim.x, dim.y, ro.outlineWidth);
				}
				else if (ro.rt == RenderType::IMAGE)
				{
					brush.setColorRGB(1, 1, 1);
					brush.drawImage(pos, dim, *ro.image);
				}
				else
					throw bbe::IllegalArgumentException();
				if (ro.outlineWidth > 0)
				{
					brush.setColorRGB(ro.fillColor);
					if (ro.rt == RenderType::BOX)
						brush.fillRect  (pos.x + ro.outlineWidth, pos.y + ro.outlineWidth, dim.x - ro.outlineWidth * 2, dim.y - ro.outlineWidth * 2);
					else if (ro.rt == RenderType::CIRCLE)
						brush.fillCircle(pos.x + ro.outlineWidth, pos.y + ro.outlineWidth, dim.x - ro.outlineWidth * 2, dim.y - ro.outlineWidth * 2);
					else if (ro.rt == RenderType::TEXT)
						{ /*Do nothing*/ }
					else if (ro.rt == RenderType::LINE)
						{ /*Do nothing*/ }
					else if (ro.rt == RenderType::ARROW)
						{ /*Do nothing*/ }
					else if (ro.rt == RenderType::IMAGE)
						{ /*Do nothing*/ }
					else
						throw bbe::IllegalArgumentException();
				}
				if (ro.text.getLength() > 0 && ro.font && ro.showText && ro.rt != RenderType::IMAGE)
				{
					brush.setColorRGB(ro.textColor);
					brush.fillText(pos.x + textMargin + ro.outlineWidth - ro.textBoundingBox.getX() - ro.textBoundingBox.getWidth () / 2 + (dim.x - textMargin * 2 - ro.outlineWidth * 2) / 2
						         , pos.y + textMargin + ro.outlineWidth - ro.textBoundingBox.getY() - ro.textBoundingBox.getHeight() / 2 + (dim.y - textMargin * 2 - ro.outlineWidth * 2) / 2
						         , ro.text, *ro.font);
				}
			}
			//for (const RenderObject& ro : tokenizer->tokens[i].renderObjects)
			//{
			//	// Debug drawing
			//	bbe::Vector2 pos = ro.getPos(animationTime);
			//	brush.setColorRGB(1, 0, 0, 1);
			//	brush.fillText(pos.x, pos.y, ro.name, fonts[10]);
			//}
		}
	}

	if (childSlides.getLength() > 0)
	{
		// TODO this is only correct for horizontal split!
		brush.setColorRGB(0.5, 0.5, 0.5, 1);
		brush.fillLine(1280 / 2, 0, 1280 / 2, 800, 3);
	}

	for (size_t i = 0; i < childSlides.getLength(); i++)
	{
		childSlides[i].draw(brush, bbe::Color(0, 0, 0, 0));
	}
}

void Slide::addType(const bbe::String& type)
{
	additionalTypes.add(type);
}

void Slide::addValue(const bbe::String& value)
{
	additionalValues.add(value);
}

void Slide::next()
{
	if (currentEntry < getAmountOfEntries())
	{
		animationTime = 0;
		currentEntry++;
	}
	else
	{
		for (size_t i = 0; i < childSlides.getLength(); i++)
		{
			if (childSlides[i].hasNext())
			{
				childSlides[i].next();
				break;
			}
		}
	}
	std::cout << "Now at Entry: " << currentEntry << std::endl;
}

bool Slide::hasNext() const
{
	if (currentEntry < getAmountOfEntries()) return true;

	for (size_t i = 0; i < childSlides.getLength(); i++)
	{
		if (childSlides[i].hasNext()) return true;
	}

	return false;
}

void Slide::prev()
{
	bool childPreved = false;
	for (size_t i = childSlides.getLength() - 1; i != (size_t)-1; i--)
	{
		if (childSlides[i].currentEntry > -1 && !childSlides[i].complete)
		{
			childSlides[i].currentEntry--;
			childPreved = true;
			break;
		}
	}
	if (currentEntry > 0 && !childPreved)
	{
		animationTime = 0;
		currentEntry--;
		while (currentEntry > 0 && isEntryAutoNext(currentEntry))
		{
			currentEntry--;
		}
	}
	std::cout << "Now at Entry: " << currentEntry << std::endl;
}

bool Slide::hasPrev() const
{
	if (currentEntry > 0) return true;

	for (size_t i = 0; i < childSlides.getLength(); i++)
	{
		if (childSlides[i].currentEntry > (childSlides[i].complete ? 0 : -1)) return true;
	}

	return false;
}

void Slide::compile()
{
	if (!tokenizer->isTextBased()) return;

	tokenizer->tokenize(text, getFont());

	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		for (size_t k = 0; k < tokenizer->tokens[i].chars.getLength(); k++)
		{
			const bbe::Rectangle currentAabb = bbe::Rectangle(tokenizer->tokens[i].chars[k].pos, getFont().getDimensions(tokenizer->tokens[i].chars[k].c));
			if (k == 0)
			{
				tokenizer->tokens[i].aabb = currentAabb;
			}
			else
			{
				tokenizer->tokens[i].aabb = tokenizer->tokens[i].aabb.combine(currentAabb);
			}
		}
	}
	tokenizer->determineTokenTypes(additionalTypes, additionalValues);
	tokenizer->animateTokens();
	if (complete)
	{
		for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
		{
			tokenizer->tokens[i].showIndex = 0;
		}
	}


	int32_t highestShowIndex = 0;
	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		if (tokenizer->tokens[i].showIndex > highestShowIndex) highestShowIndex = tokenizer->tokens[i].showIndex;
	}
	amountOfEntries = highestShowIndex;

	dirty = false;
}

bbe::String Slide::getPowerPointContent(int32_t index)
{
	const bbe::String templateContent = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateContent.xml");
	const int32_t slideWidth = 12192000;
	const int32_t slideHeight = 6858000;

	bbe::String retVal = "";

	int32_t id = 1;

	for (size_t i = 0; i < tokenizer->tokens.getLength(); i++)
	{
		if (tokenizer->tokens[i].showIndex > index) continue;
		for(size_t k = 0; k<tokenizer->tokens[i].chars.getLength(); k++)
		{
			id++;
			const bbe::Vector2 offset = -textAabb.getPos() - textAabb.getDim() * 0.5f + bbe::Vector2(1280, 720) * 0.5f - bbe::Vector2(0, getFont().getPixelsFromLineToLine());
			const bbe::Vector2 screenSpace = tokenizer->tokens[i].chars[k].powerPointPos + offset;
			const bbe::Vector2 normSpace = screenSpace / bbe::Vector2(1280, 720);
			const bbe::Vector2 powerPointSpace = bbe::Vector2(normSpace.x * slideWidth, normSpace.y * slideHeight);

			// The multiplication of the font size of stb to power point format.
			// I don't really have a clue why it is exactly this number, but I
			// did some pixel measurements and it seems to be correct.
			constexpr uint32_t stbToPowerPointFontSize = 79;

			const bbe::String text = bbe::String::fromCodePoint(tokenizer->tokens[i].chars[k].c)
				.replace("&", "&amp;")
				.replace("<", "&lt;")
				.replace(">", "&gt;");

			bbe::Color color = tokenTypeToColor(tokenizer->tokens[i].type);
			if (!(tokenizer->tokens[i].showIndex == index || (index == getAmountOfEntries() && hasFinalBrightState())))
			{
				color *= 0.8;
			}

			retVal += templateContent
				.replace("%%%XPOS%%%", bbe::String((int32_t)(powerPointSpace.x)))
				.replace("%%%YPOS%%%", bbe::String((int32_t)(powerPointSpace.y)))
				.replace("%%%TEXT%%%", text)
				.replace("%%%ID%%%", bbe::String(id))
				.replace("%%%SIZE%%%", bbe::String(getFont().getFontSize() * stbToPowerPointFontSize))
				.replace("%%%COLOR%%%", color.toHex());
		}
	}

	return retVal;
}

bbe::Font& Slide::getFont()
{
	if (selectedFont == nullptr)
	{
		selectedFont = &fonts[0];
		scrollingAllowed = true;
		for (size_t i = 0; i < fonts.getLength(); i++)
		{
			if (forcedFontSize != 0)
			{
				if (fonts[i].getFontSize() != forcedFontSize)
				{
					continue;
				}
			}
			bbe::List<bbe::Vector2> renderPositions = fonts[i].getRenderPositions(bbe::Vector2(0, 0), text);
			bbe::Rectangle textAabb = bbe::Rectangle(renderPositions[0], getFont().getDimensions(text.getCodepoint(0)));

			for (size_t k = 1; k < renderPositions.getLength(); k++)
			{
				textAabb = textAabb.combine(bbe::Rectangle(renderPositions[k], getFont().getDimensions(text.getCodepoint(k))));
			}
			
			if (i == 0)
			{
				// Make sure we have an Aabb, even if it is large.
				this->textAabb = textAabb;
			}
			else if (i == 1)
			{
				scrollingAllowed = false;
			}

			if ((textAabb.getHeight() < screenPosition.getHeight() && textAabb.getWidth() < screenPosition.getWidth()) || forcedFontSize != 0)
			{
				selectedFont = &fonts[i];
				this->textAabb = textAabb;
			}

			if (textAabb.getHeight() > screenPosition.getHeight() - 50 || textAabb.getWidth() > screenPosition.getWidth() - 50) break;
		}
		std::cout << "Picked font size: " << selectedFont->getFontSize() << std::endl;
	}

	return *selectedFont;
}

void Slide::moveFrom(Slide&& other)
{
	currentEntry        = std::move(other.currentEntry);
	amountOfEntries     = std::move(other.amountOfEntries);
	dirty               = std::move(other.dirty);
	selectedFont        = std::move(other.selectedFont);
	forcedFontSize      = std::move(other.forcedFontSize);
	additionalTypes     = std::move(other.additionalTypes);
	additionalValues    = std::move(other.additionalValues);
	textAabb            = std::move(other.textAabb);
	text                = std::move(other.text);
	tokenizer           = std::move(other.tokenizer);
	scrollValue         = std::move(other.scrollValue);
	scrollingAllowed    = std::move(other.scrollingAllowed);
	complete            = std::move(other.complete);
	screenPosition      = std::move(other.screenPosition);
	childSlides         = std::move(other.childSlides);
	brightStateOverride = std::move(other.brightStateOverride);
}

void Slide::copyFrom(const Slide& other)
{
	currentEntry        = other.currentEntry;
	amountOfEntries     = other.amountOfEntries;
	dirty               = other.dirty;
	selectedFont        = other.selectedFont;
	forcedFontSize      = other.forcedFontSize;
	additionalTypes     = other.additionalTypes;
	additionalValues    = other.additionalValues;
	textAabb            = other.textAabb;
	text                = other.text;
	tokenizer           = other.tokenizer;
	scrollValue         = other.scrollValue;
	scrollingAllowed    = other.scrollingAllowed;
	complete            = other.complete;
	screenPosition      = other.screenPosition;
	childSlides         = other.childSlides;
	brightStateOverride = other.brightStateOverride;
}

bool Slide::isEntryAutoNext(int32_t entry) const
{
	for (const Token& t : tokenizer->tokens)
	{
		if (t.showIndex == entry && t.autoNext) return true;
	}
	return false;
}

void Token::submit(bbe::List<Token>& tokens)
{
	decltype(this->renderObjects) ros;
	const float animationMult = animationMultiplier;
	if (chars.getLength() > 0 || renderObjects.getLength() > 0 || lines.getLength() > 0)
	{
		ros = this->renderObjects;
		tokens.add(*this);
	}
	*this = Token();
	this->renderObjects = ros;
	for (RenderObject& ro : renderObjects)
	{
		ro.exhaustAnimations();
	}
	for (RenderObject& ro : renderObjects)
	{
		ro.startAnim = StartAnimation::NONE;
	}
	animationMultiplier = animationMult;
}

bbe::List<size_t> Token::getRenderObjectIndices(const bbe::List<bbe::String>& names) const
{
	bbe::List<size_t> indices;
	for (size_t i = 0; i < renderObjects.getLength(); i++)
	{
		for (size_t k = 0; k < names.getLength(); k++)
		{
			if (renderObjects[i].name == names[k])
			{
				indices.add(i);
			}
		}
	}
	return indices;
}

void CppTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);

	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
	}

	Token currentToken;

	bool lineComment = false;
	bool isString = false;
	int64_t multiComment = 0;
	int64_t forceAdd = 0;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		forceAdd--;
		if (text[i] == '\n') lineComment = false;
		if (text.isTextAtLocation("//", i)) lineComment = true;
		if (text.isTextAtLocation("/*", i)) multiComment++;
		if (text.isTextAtLocation("*/", i))
		{
			multiComment--;
			if (multiComment == 0)
			{
				forceAdd = 2;
			}
		}
		if (text.isTextAtLocation("->", i))
		{
			currentToken.submit(tokens);
			forceAdd = 2;
		}
		if (i >= 2 && text.isTextAtLocation("->", i - 2))
		{
			currentToken.submit(tokens);
		}
		if (text[i] == '"') isString = !isString;

		if (!isString)
		{
			if (singleSignTokens.contains(text[i]) && !lineComment && multiComment == 0 && forceAdd <= 0)
			{
				currentToken.submit(tokens);
			}
			if ((text.isTextAtLocation("::", i) || (i >= 2 && text.isTextAtLocation("::", i - 2))) && !lineComment && multiComment == 0)
			{
				currentToken.submit(tokens);
			}
			if (text.isTextAtLocation(",", i))
			{
				currentToken.submit(tokens);
			}
		}
		if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
		{
			Char c;
			c.c = text.getCodepoint(i);
			c.pos = renderPositions[i];
			c.powerPointPos = powerPointPositions[i];

			currentToken.chars.add(c);
			currentToken.text += text[i];
		}
		else
		{
			if (!lineComment && multiComment == 0 && forceAdd <= 0 && !isString) currentToken.submit(tokens);
		}
		if (singleSignTokens.contains(text[i]) && !lineComment && multiComment == 0 && forceAdd <= 0 && !isString)
		{
			currentToken.submit(tokens);
		}
	}

	if (currentToken.chars.getLength() > 0)
	{
		tokens.add(currentToken);
	}
}

void CppTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String> &additionalValues)
{
	const bbe::List<bbe::String> keywords = { "constexpr", "class", "public", "private", "protected", "public:", "private:", "protected:", "virtual", "override", "if", "else", "alignas", "alignof", "delete", "operator", "sizeof", "template", "typename", "struct", "return", "noexcept", "this", "void", "int", "float", "double", "co_yield", "co_return", "co_await", "true", "false", "auto", "char", "int64_t", "const", "mutable", "while", "for", "do", "noexcept", "static", "volatile", "throw", "decltype", "typeof"};
	const bbe::List<bbe::String> preprocessor = { "#include", "#define", "#ifdef", "#endif" };
	bbe::List<bbe::String> types = { "int", "char", "float", "bool", "__m128", "__m256i", "__m256d"};
	types.addList(additionalTypes);
	types.add("uint32_t");
	types.add("int32_t");
	types.add("T");
	types.add("GLfloat");
	types.add("GLint");
	types.add("GLuint");
	types.add("GL_FLOAT");
	types.add("GL_FALSE");
	types.add("GL_ARRAY_BUFFER");
	types.add("GL_ELEMENT_ARRAY_BUFFER");
	types.add("GL_COLOR_BUFFER_BIT");
	types.add("GL_TRIANGLES");
	types.add("GL_UNSIGNED_INT");
	types.add("GL_DEPTH_BUFFER_BIT");
	types.add("GL_STATIC_DRAW");
	types.add("GL_VERTEX_SHADER");
	types.add("GL_FRAGMENT_SHADER");
	types.add("NULL");
	types.add("GLEW_OK");
	bbe::List<bbe::String> values = {};
	values.addList(additionalValues);

	bool change = true;
	bool withinInclude = false;
	while (change)
	{
		change = false;
		for (size_t i = 0; i < tokens.getLength(); i++)
		{
			if (tokens[i].type != TokenType::unknown) continue;

			if (tokens[i].text.endsWith(">") || tokens[i].text.trim().endsWith("\""))
			{
				withinInclude = false;
			}

			bool didChangeThisToken = true;
			if (tokens[i].text.startsWith("//")
				|| tokens[i].text.startsWith("/*"))
			{
				tokens[i].type = TokenType::comment;
			}
			else if (preprocessor.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::preprocessor;
			}
			else if (withinInclude || (i >= 2 && (tokens[i - 1].text == "<" || tokens[i - 1].text == "\"") && tokens[i - 2].text == "#include"))
			{
				withinInclude = true;
				tokens[i].type = TokenType::include_path;
			}
			else if (tokens[i].text.startsWith("\""))
			{
				tokens[i].type = TokenType::string;
			}
			else if (keywords.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::keyword;
			}
			else if (tokens[i].text.isNumber()
				|| (tokens[i].text == "." && i > 0 && tokens[i - 1].type == TokenType::number)
				|| (tokens[i].text.endsWith("f") && i > 0 && tokens[i - 1].type == TokenType::number))
			{
				tokens[i].type = TokenType::number;
			}
			else if ((tokens[i].text == "::") || (tokens[i].text == "->") || (tokens[i].text == "%") || (tokens[i].text == "-") || (tokens[i].text == "/") || (tokens[i].text == ",") || (tokens[i].text.getLength() == 1 && singleSignTokens.contains(tokens[i].text[0])))
			{
				tokens[i].type = TokenType::punctuation;
			}
			else if (types.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::type;
			}
			else if (i != tokens.getLength() - 1 && tokens[i + 1].text == "(")
			{
				tokens[i].type = TokenType::function;
			}
			else if (i != tokens.getLength() - 1 && tokens[i + 1].text == "::")
			{
				tokens[i].type = TokenType::namespace_;
			}
			else if (tokens[i].text == ":")
			{
				tokens[i].type = TokenType::punctuation;
			}
			else if ((i > 0 && (tokens[i - 1].text == "struct"
				|| tokens[i - 1].text == "typename"
				|| tokens[i - 1].text == "class"
				|| tokens[i - 1].text == "::"))
				|| types.contains(tokens[i].text))
			{
				tokens[i].type = TokenType::type;
				if (!types.contains(tokens[i].text)) types.add(tokens[i].text);
			}
			else if ((i > 0 && tokens[i - 1].type == TokenType::type)
				|| (i > 0 && types.contains(tokens[i - 1].text))
				|| (i > 0 && tokens[i - 1].text == "=")
				|| (i > 0 && tokens[i - 1].text == "return")
				|| (i < tokens.getLength() - 1 && tokens[i + 1].text == "=")
				|| (i < tokens.getLength() - 1 && tokens[i + 1].text == ")")
				|| (values.contains(tokens[i].text))
				|| (i > 1 && tokens[i - 2].type == TokenType::type && (tokens[i-1].text == "*" || tokens[i-1].text == "&")))
			{
				tokens[i].type = TokenType::value;
				if (!values.contains(tokens[i].text)) values.add(tokens[i].text);
			}
			else if (i > 0 && tokens[i - 1].text == "#ifdef")
			{
				tokens[i].type = TokenType::preprocessor;
			}
			else
			{
				didChangeThisToken = false;
			}

			if (didChangeThisToken)
			{
				change = true;
			}
		}
	}
}

void CppTokenizer::animateTokens()
{
	int32_t currentShowIndex = -1;
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		if (tokens[i].showIndex != -1)
		{
			continue;
		}

		if (i > 0 && tokens[i - 1].type == TokenType::type && tokens[i].text == "<" && (i == tokens.getLength() - 1 || tokens[i + 1].text != "<"))
		{
			int32_t depth = 0;
			for (size_t k = i; k < tokens.getLength(); k++)
			{
				tokens[k].showIndex = currentShowIndex;
				if (tokens[k].text == "<" && (k == tokens.getLength() - 1 || tokens[k + 1].text != "<"))
				{
					depth++;
				}
				else if (tokens[k].text == ">")
				{
					depth--;
					if (depth == 0)
					{
						break;
					}
				}
			}
		}
		else if (tokens[i].text == "cout" || tokens[i].text == "endl")
		{
			tokens[i].showIndex = currentShowIndex;
		}
		else if (tokens[i].type == TokenType::punctuation)
		{
			tokens[i].showIndex = currentShowIndex;

			if (tokens[i].text == "{")
			{
				int32_t depth = 0;
				for (size_t k = i; k < tokens.getLength(); k++)
				{
					if (tokens[k].text == "{")
					{
						depth++;
					}
					else if (tokens[k].text == "}")
					{
						depth--;
						if (depth == 0)
						{
							tokens[k].showIndex = currentShowIndex;
							if (k < tokens.getLength() - 1 && tokens[k + 1].text == ";")
							{
								tokens[k + 1].showIndex = currentShowIndex;
							}
							break;
						}
					}
				}
			}

			if (tokens[i].text == "(")
			{
				int32_t depth = 0;
				for (size_t k = i; k < tokens.getLength(); k++)
				{
					if (tokens[k].text == "(")
					{
						depth++;
					}
					else if (tokens[k].text == ")")
					{
						depth--;
						if (depth == 0)
						{
							tokens[k].showIndex = currentShowIndex;
							break;
						}
					}
				}
			}
		}
		else if (tokens[i].text == "for")
		{
			currentShowIndex++;
			tokens[i].showIndex = currentShowIndex;

			currentShowIndex++;

			for (size_t k = i + 1; k < tokens.getLength(); k++)
			{
				if (tokens[k].text == "(" || tokens[k].text == ")")
				{
					tokens[k].showIndex = tokens[i].showIndex;
				}
				else
				{
					tokens[k].showIndex = currentShowIndex;
				}

				if (tokens[k].text == ";")
				{
					currentShowIndex++;
				}

				if (tokens[k].text == ")")
				{
					break;
				}
			}
		}
		else if (tokens[i].type == TokenType::include_path)
		{
			tokens[i].showIndex = currentShowIndex;
		}
		else if (tokens[i].type == TokenType::number && i > 0 && tokens[i - 1].type == TokenType::number)
		{
			tokens[i].showIndex = currentShowIndex;
		}
		else
		{
			currentShowIndex++;
			tokens[i].showIndex = currentShowIndex;
		}
	}
}

bool CppTokenizer::hasFinalBrightState()
{
	return true;
}

void LineTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::List<bbe::Vector2> renderPositions     = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);
	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
	}

	Token currentToken;

	for (size_t i = 0; i < renderPositions.getLength(); i++)
	{
		if (text[i] == '\n' && (i == 0 || text[i-1] != '\\'))
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

struct Array
{
	bbe::String name;
	bbe::List<bbe::String> targets;
};

bbe::List<size_t> getValidIndices(const bbe::List<Array>& arrays, const Token& token, const bbe::String& identifier)
{
	bbe::List<size_t> retVal;

	for (const Array& arr : arrays)
	{
		if (identifier == arr.name)
		{
			for (const bbe::String& target : arr.targets)
			{
				retVal.addList(token.getRenderObjectIndices({ target }));
			}
		}
	}

	retVal.addList(token.getRenderObjectIndices({ identifier }));

	return retVal;
}

BrotDownTokenizer::BrotDownTokenizer(const bbe::String& parentPath)
	: parentPath(parentPath)
{

}

#define ForEachValidIdentifiers(identifier) \
for(size_t index : getValidIndices(arrays, currentToken, (identifier)))

void BrotDownTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::String noTilde = text.replace("~", "");
	const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(bbe::Vector2(0, 0), noTilde);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), noTilde, 0, false);

	Token currentToken;

	bool newLine = true;

	int32_t lineNumber = 1;

	bool lineStarted = false;
	bbe::Vector2 startLine;

	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float outlineWidth = 3;
	int32_t repeats = 1;
	StartAnimation startAnim = StartAnimation::NONE;
	bool nextToMode = false;

	bbe::List<Array> arrays;
	bbe::List<bbe::String> autoArrs;

	for (size_t i = 0, posAccess = 0; i < text.getLength(); i++, posAccess++)
	{
		if(newLine && text[i] == '!')
		{
			int64_t commandEnd = text.search("\n", i);
			if (commandEnd < 0) commandEnd = text.getLength();
			bbe::String command = text.substring(i + 1, commandEnd);
			i += command.getLength() + 1;
			posAccess += command.getLength() + 1;
			const int64_t textStart = command.search("\"");
			const int64_t textEnd = command.searchLast("\"");
			bbe::String text;

			if (command.startsWith("//")) continue;

			if (textStart != -1 && textEnd != -1)
			{
				if (textStart != textEnd)
				{
					text = command.substring(textStart + 1, textEnd);
					command = command.substring(0, textStart - 1) + command.substring(textEnd + 1, command.getLength());
				}
				else
				{
					std::cout << "Found only a single \"" << std::endl;
					throw bbe::IllegalArgumentException();
				}
			}
			command = command.trim();
			std::cout << "COMMAND: " << command << std::endl;

			const auto commandTokens = command.split(" ", false);
			for (const bbe::String& token : commandTokens)
			{
				std::cout << "\t" << token << std::endl;
			}
			int32_t nextRepeats = 1;

			for(int32_t repeat = 0; repeat<repeats; repeat++)
			{
				bool commandRecognized = false;

				if (commandTokens[0] == "Next")
				{
					currentToken.submit(this->tokens);
					commandRecognized = true;
				}
				else if (commandTokens[0] == "AutoNext")
				{
					currentToken.autoNext = true;
					currentToken.submit(this->tokens);
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Teleport")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						currentToken.renderObjects[index].x = commandTokens[2].toFloat();
						currentToken.renderObjects[index].y = commandTokens[3].toFloat();
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Destroy")
				{
					auto indices = getValidIndices(arrays, currentToken, commandTokens[1]);
					indices.sort();
					for (size_t i = indices.getLength() - 1; i != (size_t)-1; i--)
					{
						currentToken.renderObjects.removeIndex(indices[i]);
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Move")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						MoveAnimation anim;

						if (commandTokens[2] == "NextTo")
						{
							anim = MoveAnimation(x + width, y);
						}
						else
						{
							anim = MoveAnimation(commandTokens[2].toFloat(), commandTokens[3].toFloat());
						}

						currentToken.renderObjects[index].animations.add(anim);
						x = anim.targetX;
						y = anim.targetY;
						width = currentToken.renderObjects[index].width;
						height = currentToken.renderObjects[index].height;
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Bezier")
				{

					ForEachValidIdentifiers(commandTokens[1])
					{
						if (commandTokens.getLength() >= 8)
						{
							currentToken.renderObjects[index].animations.add(MoveAnimation(
								commandTokens[2].toFloat(),
								commandTokens[3].toFloat(),
								commandTokens[4].toFloat(),
								commandTokens[5].toFloat(),
								commandTokens[6].toFloat(),
								commandTokens[7].toFloat()
							));
						}
						else
						{
							currentToken.renderObjects[index].animations.add(MoveAnimation(
								commandTokens[2].toFloat(),
								commandTokens[3].toFloat(),
								commandTokens[4].toFloat(),
								commandTokens[5].toFloat()
							));
						}
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Translate")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						currentToken.renderObjects[index].animations.add(MoveAnimation(
							currentToken.renderObjects[index].x + commandTokens[2].toFloat(),
							currentToken.renderObjects[index].y + commandTokens[3].toFloat()
						));
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "StartAnim")
				{
						 if (commandTokens[1] == "None")   startAnim = StartAnimation::NONE;
					else if (commandTokens[1] == "ZoomIn") startAnim = StartAnimation::ZOOM_IN;
					else throw bbe::IllegalArgumentException();
				
					commandRecognized = true;
				}
				else if (commandTokens[0] == "HideText")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						currentToken.renderObjects[index].showText = false;
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "ShowText")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						currentToken.renderObjects[index].showText = true;
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Color")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						currentToken.renderObjects[index].fillColor = bbe::Color(
							commandTokens[2].toFloat(),
							commandTokens[3].toFloat(),
							commandTokens[4].toFloat()
						);
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "OutlineColor")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						currentToken.renderObjects[index].outlineColor = bbe::Color(
							commandTokens[2].toFloat(),
							commandTokens[3].toFloat(),
							commandTokens[4].toFloat()
						);
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "TextColor")
				{
					ForEachValidIdentifiers(commandTokens[1])
					{
						if (commandTokens[2] == "Default")
						{
							currentToken.renderObjects[index].textColor = RenderObject::defaultTextColor;
						}
						else
						{
							currentToken.renderObjects[index].textColor = bbe::Color(
								commandTokens[2].toFloat(),
								commandTokens[3].toFloat(),
								commandTokens[4].toFloat()
							);
						}
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Gradient")
				{
					float min = 1000000000;
					float max = -1000000000;
					ForEachValidIdentifiers(commandTokens[1])
					{
						float val = currentToken.renderObjects[index].text.toFloat();
						if (val < min) min = val;
						if (val > max) max = val;
					}
					const float diff = max - min;
					bbe::Vector3 c1 = bbe::Vector3(
						commandTokens[2].toFloat(),
						commandTokens[3].toFloat(),
						commandTokens[4].toFloat()
					);
					bbe::Vector3 c2 = bbe::Vector3(
						commandTokens[5].toFloat(),
						commandTokens[6].toFloat(),
						commandTokens[7].toFloat()
					);
					ForEachValidIdentifiers(commandTokens[1])
					{
						float val = currentToken.renderObjects[index].text.toFloat();
						float percentage = (val - min) / diff;
						bbe::Vector3 vc = bbe::Math::interpolateLinear(c1, c2, percentage);
						currentToken.renderObjects[index].fillColor = bbe::Color(vc.x, vc.y, vc.z);
					}
					commandRecognized = true;
				}
				else if (commandTokens[0] == "MakeArr")
				{
					Array arr;
					arr.name = commandTokens[1];
					for (size_t i = 2; i < commandTokens.getLength(); i++)
					{
						arr.targets.add(commandTokens[i]);
					}
					arrays.add(arr);
					commandRecognized = true;
				}
				else if (commandTokens[0] == "AutoArr" || commandTokens[0] == "AutoArray")
				{
					Array arr;
					arr.name = commandTokens[1];
					autoArrs.add(arr.name);
					arrays.add(arr);
					commandRecognized = true;
				}
				else if (commandTokens[0] == "StopAutoArr" || commandTokens[0] == "StopAutoArray")
				{
					autoArrs.clear();
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Repeat")
				{
					nextRepeats = commandTokens[1].toLong();
					commandRecognized = true;
				}
				else if (commandTokens[0] == "AnimMult")
				{
					currentToken.animationMultiplier = commandTokens[1].toFloat();
					commandRecognized = true;
				}
				else if (commandTokens[0] == "Sort"
					  || commandTokens[0] == "Shuffle"
					  || commandTokens[0] == "Partition")
				{
					bbe::List<size_t> indices = getValidIndices(arrays, currentToken, commandTokens[1]);
					const bbe::List<size_t> unsortedIndices = indices;
					const bbe::Vector2 startPos = currentToken.renderObjects[indices.first()].getPos(0);
					const bbe::Vector2 endPos   = currentToken.renderObjects[indices.last ()].getPos(0);
					const bbe::Vector2 diffPos  = endPos - startPos;
					if (commandTokens[0] == "Sort")
					{
						indices.sort([&](const size_t& a, const size_t& b) {
							return currentToken.renderObjects[a].text.toFloat() < currentToken.renderObjects[b].text.toFloat();
						});
					}
					else if (commandTokens[0] == "Shuffle")
					{
						indices.shuffle(17);
					}
					else if (commandTokens[0] == "Partition")
					{
						indices.partition([&](const size_t& i) {
							return currentToken.renderObjects[i].text.toLong() % 2 == 0;
						});
					}

					for (size_t index = 0; index < indices.getLength(); index++)
					{
						const float percentage = (float)index / (indices.getLength() - 1.f);
						const bbe::Vector2 pos = startPos + diffPos * percentage;
						const bbe::Vector2 controlOffset = diffPos.rotate90CounterClockwise().normalize() * ((float)indices[index] - (float)unsortedIndices[index]) * 50;
						const bbe::Vector2 control = currentToken.renderObjects[indices[index]].getPos(0) + controlOffset;
						const bbe::Vector2 control2 = pos + controlOffset;
						currentToken.renderObjects[indices[index]].animations.add(MoveAnimation(pos.x, pos.y, control.x, control.y, control2.x, control2.y));
					}
					commandRecognized = true;
				}

				size_t accessIndex = 0;
				bbe::String name = "ANON-OBJECT-" + bbe::String(lineNumber) + "-" + bbe::String(repeat);
				if (commandTokens.getLength() > 1 && commandTokens[1] == "=")
				{
					name = commandTokens[0];
					accessIndex += 2;
				}
				if (!commandRecognized &&
					commandTokens.getLength() > accessIndex && 
					(
						   commandTokens[accessIndex] == "Box"
						|| commandTokens[accessIndex] == "Circle"
						|| commandTokens[accessIndex] == "Text"
						|| commandTokens[accessIndex] == "Line"
						|| commandTokens[accessIndex] == "Arrow"
						|| commandTokens[accessIndex] == "Image"
					))
				{
					RenderType rt = RenderType::UNKNOWN;
						 if (commandTokens[accessIndex] == "Box")    rt = RenderType::BOX;
					else if (commandTokens[accessIndex] == "Circle") rt = RenderType::CIRCLE;
					else if (commandTokens[accessIndex] == "Text")   rt = RenderType::TEXT;
					else if (commandTokens[accessIndex] == "Line")   rt = RenderType::LINE;
					else if (commandTokens[accessIndex] == "Arrow")  rt = RenderType::ARROW;
					else if (commandTokens[accessIndex] == "Image")  rt = RenderType::IMAGE;
					else throw bbe::IllegalArgumentException();

					accessIndex++;
					if (commandTokens.getLength() > accessIndex)
					{
						if (commandTokens[accessIndex] == "NextTo")
						{
							accessIndex++;
							nextToMode = true;
						}
						else
						{
							x = commandTokens[accessIndex].toFloat();
							accessIndex++;
							nextToMode = false;
							if (commandTokens.getLength() > accessIndex)
							{
								y = commandTokens[accessIndex].toFloat();
								accessIndex++;
							}
						}
					}
					if (nextToMode)
					{
						x += width;
					}
					if (commandTokens.getLength() > accessIndex)
					{
						width = commandTokens[accessIndex].toFloat();
						accessIndex++;
					}
					if (commandTokens.getLength() > accessIndex)
					{
						height = commandTokens[accessIndex].toFloat();
						accessIndex++;
					}
					if (commandTokens.getLength() > accessIndex)
					{
						outlineWidth = commandTokens[accessIndex].toFloat();
						accessIndex++;
					}

					for (const bbe::String& autoArr : autoArrs)
					{
						Array* arr = arrays.find([&](const Array& arr) { return arr.name == autoArr; });
						if (arr)
						{
							arr->targets.add(name);
						}
						else
						{
							std::cout << "Something strange happened..." << std::endl;
							throw bbe::IllegalStateException();
						}
					}

					bbe::FittedFont ff;
					if (text.getLength() > 0)
					{
						ff = bbe::Font::getBestFittingFont(fonts, text, { width - textMargin * 2 - outlineWidth * 2, height - textMargin * 2 - outlineWidth * 2 });
						if (ff.font)
						{
							text = ff.string;
						}
					}

					RenderObject ro = RenderObject{ name, rt, startAnim, x, y, width, height, outlineWidth, text, ff.font };
					if (ff.font)
					{
						ro.textBoundingBox = ff.font->getBoundingBox(text);
					}

					if (rt == RenderType::TEXT)
					{
						ro.fillColor    = bbe::Color(0, 0, 0, 0);
						ro.outlineColor = bbe::Color(0, 0, 0, 0);
						ro.textColor    = bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
					}
					else if (rt == RenderType::IMAGE)
					{
						ro.image.reset(new bbe::Image((parentPath + ro.text).getRaw()));
					}

					currentToken.renderObjects.add(ro);
					commandRecognized = true;
				}

				if(!commandRecognized)
				{
					std::cout << "Got illegal argument: " << command << std::endl;
					throw bbe::IllegalArgumentException();
				}
			}

			lineNumber++;
			repeats = nextRepeats;
		}
		else
		{
			if (text[i] == '\n' && (i == 0 || text[i - 1] != '\\'))
			{
				currentToken.submit(tokens);
				lineStarted = false;
				newLine = true;
				lineNumber++;
			}
			else
			{
				newLine = false;
			}

			if (text[i] == '~')
			{
				if (!lineStarted)
				{
					startLine = renderPositions[posAccess];
					startLine.y += font.getDimensions(noTilde[posAccess]).y;
					startLine.y -= font.getDimensions('A').y / 2;
					posAccess--;
				}
				else
				{
					posAccess--;
					bbe::Vector2 stopLine = renderPositions[posAccess];
					stopLine += font.getDimensions(noTilde[posAccess]);
					stopLine.y -= font.getDimensions('A').y / 2;
				
					currentToken.lines.add(bbe::Line2(startLine, stopLine));
				}

				lineStarted = !lineStarted;
			}

			if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r' && text[i] != '~' && !(i != renderPositions.getLength() - 1 && text[i] == '\\' && text[i + 1] == '\n'))
			{
				Char c;
				c.c = text.getCodepoint(i);
				c.pos = renderPositions[posAccess];
				c.powerPointPos = powerPointPositions[posAccess];

				currentToken.chars.add(c);
				currentToken.text += bbe::String::fromCodePoint(c.c);
			}
		}
	}
	currentToken.submit(tokens);
}

void BrotDownTokenizer::determineTokenTypes(const bbe::List<bbe::String>& additionalTypes, const bbe::List<bbe::String>& additionalValues)
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].type = TokenType::text;
	}
}

void BrotDownTokenizer::animateTokens()
{
	for (size_t i = 0; i < tokens.getLength(); i++)
	{
		tokens[i].showIndex = i;
	}
}

bool BrotDownTokenizer::hasFinalBrightState()
{
	return false;
}

void AsmTokenizer::tokenize(const bbe::String& text, const bbe::Font& font)
{
	const bbe::List<bbe::Vector2> renderPositions     = font.getRenderPositions(bbe::Vector2(0, 0), text);
	const bbe::List<bbe::Vector2> powerPointPositions = font.getRenderPositions(bbe::Vector2(0, 0), text, 0, false);
	if (renderPositions.getLength() != text.getLength())
	{
		throw bbe::IllegalStateException();
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

void PngTokenizer::loadImage(const bbe::String& path)
{
	Token t;
	t.images.add(bbe::Image(path));

	tokens.add(t);
}

void SlideShow::update(PresentationControl pc, float scrollValue, float timeSinceLastFrame)
{
	if (pc == PresentationControl::none)
	{
		slides[currentSlide].update(pc, scrollValue, timeSinceLastFrame);
	}
	else if (pc == PresentationControl::previous)
	{
		if (slides[currentSlide].isFirstEntry())
		{
			if (currentSlide > 0)
			{
				currentSlide--;
				slides[currentSlide].animationTime = 0;
			}
		}
		else
		{
			slides[currentSlide].update(pc, scrollValue, timeSinceLastFrame);
		}
	}
	else if (pc == PresentationControl::next)
	{
		if (slides[currentSlide].isLastEntry())
		{
			if (currentSlide < slides.getLength() - 1)
			{
				currentSlide++;
				slides[currentSlide].animationTime = 0;
			}
		}
		else
		{
			slides[currentSlide].update(pc, scrollValue, timeSinceLastFrame);
		}
	}
	else if (pc == PresentationControl::previous_slide)
	{
		if (currentSlide > 0)
		{
			currentSlide--;
			slides[currentSlide].animationTime = 0;
		}
	}
	else if (pc == PresentationControl::next_slide)
	{
		if (currentSlide < slides.getLength() - 1)
		{
			currentSlide++;
			slides[currentSlide].animationTime = 0;
		}
	}
	else
	{
		throw bbe::IllegalArgumentException();
	}
}

void SlideShow::draw(bbe::PrimitiveBrush2D& brush)
{
	slides[currentSlide].draw(brush, bgColor);
}

void SlideShow::addType(const bbe::String& type)
{
	for (size_t i = 0; i < slides.getLength(); i++)
	{
		slides[i].addType(type);
	}
}

void SlideShow::addSlide(const char* path)
{
	addSlide(Slide(path));
}

void SlideShow::addSlide(const bbe::String& path)
{
	addSlide(path.getRaw());
}

void SlideShow::addSlide(Slide& slide)
{
	slides.add(std::move(slide));
}

void SlideShow::addManifest(const char* inPath)
{
	const bbe::String path = inPath;
	const bbe::String parentPath = path.substring(0, path.searchLast("/")) + "/";
	const bbe::String fileContent = bbe::simpleFile::readFile(path);
	const bbe::DynamicArray<bbe::String> lines = fileContent.split("\n");

	uint32_t horizontalCount = 10;

	for (size_t i = 0; i < lines.getLength(); i++)
	{
		const bbe::String line = lines[i].trim();
		const bbe::DynamicArray<bbe::String> tokens = line.split(" ");
		if (tokens.getLength() == 0) continue;
		if (tokens[0].getLength() == 0) continue;
		if (tokens[0].startsWith("//")) continue;

		if (   tokens[0] == "SLIDE:" 
			|| tokens[0] == "SLIDETEXT:")
		{
			Slide slide;

			if (tokens[0] == "SLIDE:")
			{
				slide = Slide((parentPath + tokens[1]).getRaw());
			}
			else if (tokens[0] == "SLIDETEXT:")
			{
				slide = Slide(new LineTokenizer());
				slide.addText(line.getRaw() + strlen("SLIDETEXT: "));
			}
			else
			{
				throw bbe::UnknownException();
			}

			if (horizontalCount == 0)
			{
				slide.setScreenPosition(bbe::Rectangle(borderWidth, borderWidth, 1280 / 2 - 1.5 * borderWidth, 720 - 2 * borderWidth));
			}
			else if (horizontalCount == 1)
			{
				slide.setScreenPosition(bbe::Rectangle(1280 / 2 + 1.5 * borderWidth, borderWidth, 1280 / 2 - 1.5 * borderWidth, 720 - 2 * borderWidth));
				slide.currentEntry = -1;
				getLastSlide().childSlides.add(slide);
			}

			if (horizontalCount != 1)
			{
				addSlide(slide);
			}
			horizontalCount++;
		}
		else if (tokens[0] == "HORIZONTALSPLIT")
		{
			horizontalCount = 0;
		}
		else if (tokens[0] == "BORDERWIDTH:")
		{
			if (tokens[1] == "default")
			{
				borderWidth = Slide::DEFAULT_BORDERWIDTH;
			}
			else
			{
				borderWidth = tokens[1].toLong();
			}
		}
		else if (tokens[0] == "FORCEFONTSIZE:")
		{
			getLastSlide().forceFontSize(tokens[1].toLong());
		}
		else if (tokens[0].toLowerCase() == "complete")
		{
			getLastSlide().setComplete(true);
		}
		else if (tokens[0].toLowerCase() == "samedimensions")
		{
			//slides[slides.getLength() - 2].compile();
			const uint32_t fontSize = getSecondToLastSlide().getFont().getFontSize();
			getLastSlide().forceFontSize(fontSize);
			getLastSlide().getFont();
			getLastSlide().textAabb = getSecondToLastSlide().textAabb;
		}
		else if (tokens[0].toLowerCase() == "samedimensionsas:")
		{
			Slide dimSlide = Slide((parentPath + tokens[1]).getRaw());
			if (horizontalCount == 0)
			{
				dimSlide.setScreenPosition(bbe::Rectangle(borderWidth, borderWidth, 1280 / 2 - 1.5 * borderWidth, 720 - 2 * borderWidth));
			}
			else if (horizontalCount == 1)
			{
				dimSlide.setScreenPosition(bbe::Rectangle(1280 / 2 + 1.5 * borderWidth, borderWidth, 1280 / 2 - 1.5 * borderWidth, 720 - 2 * borderWidth));
			}
			const uint32_t fontSize = dimSlide.getFont().getFontSize();
			getLastSlide().forceFontSize(fontSize);
			getLastSlide().getFont();
			getLastSlide().textAabb = dimSlide.textAabb;
		}
		else if (tokens[0].toLowerCase() == "finalbrightstate")
		{
			getLastSlide().brightStateOverride = BrightStateOverride::OVERRIDE_ON;
		}
		else if (tokens[0].toLowerCase() == "nofinalbrightstate")
		{
			getLastSlide().brightStateOverride = BrightStateOverride::OVERRIDE_OFF;
		}
		else if (tokens[0] == "type:")
		{
			getLastSlide().addType(tokens[1]);
		}
		else if (tokens[0] == "value:")
		{
			getLastSlide().addValue(tokens[1]);
		}
		else
		{
			std::cout << "Found invalid token: " << tokens[0] << std::endl;
			throw bbe::UnknownException();
		}
	}
}

void SlideShow::forceFontSize(uint32_t fontSize)
{
	for (size_t i = 0; i < slides.getLength(); i++)
	{
		forceFontSize(fontSize, i);
	}
}

void SlideShow::forceFontSize(uint32_t fontSize, uint32_t slide)
{
	slides[slide].forceFontSize(fontSize);
}

void SlideShow::writeAsPowerPoint(const bbe::String& path)
{
	for (size_t i = 0; i < slides.getLength(); i++)
	{
		if (slides[i].dirty)
		{
			slides[i].compile();
		}
	}

	// TODO Remove absolute paths
	const bbe::String templateOverall = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateOverall.xml");
	const bbe::String templateSlides = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateSlides.xml");
	const bbe::String templateSlideRel = bbe::simpleFile::readFile("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/templateSlideRel.xml");
	const bbe::String templateSlideIdList = "<p:sldId id=\"%%%ID%%%\" r:id=\"rId%%%ID%%%\"/>";
	const bbe::String templateRelationships = "<Relationship Id=\"rId%%%ID%%%\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide\" Target=\"slides/slide%%%ID%%%.xml\"/>";

	bbe::String slides = "";
	bbe::String slideRels = "";
	bbe::String slideIdList = "";
	bbe::String relationships = "";

	uint32_t id = 256;

	for (int i = 0; i < this->slides.getLength(); i++)
	{
		int32_t amountOfEntries = this->slides[i].getAmountOfEntries();
		for (int32_t entry = 0; entry < amountOfEntries; entry++)
		{
			id++;
			const bbe::String stringId = bbe::String(id);

			const bbe::String powerPointContent = this->slides[i].getPowerPointContent(entry);

			slides += templateSlides.replace("%%%ID%%%", stringId)
				.replace("%%%BG_COLOR%%%", bgColor.toHex())
				.replace("%%%CONTENT%%%", powerPointContent);
			slideRels     += templateSlideRel     .replace("%%%ID%%%", stringId);
			slideIdList   += templateSlideIdList  .replace("%%%ID%%%", stringId);
			relationships += templateRelationships.replace("%%%ID%%%", stringId);
		}
	}


	bbe::String output = templateOverall
		.replace("%%%SLIDES_TEMPLATE%%%", slides)
		.replace("%%%SLIDE_ID_LIST%%%", slideIdList)
		.replace("%%%SLIDE_RELATIONSHIPS%%%", relationships)
		.replace("%%%SLIDE_REL_TEMPLATE%%%", slideRels);

	bbe::simpleFile::writeStringToFile(path, output);
}

bbe::Rectangle SlideShow::getTextAabb(uint32_t slide) const
{
	return slides[slide].textAabb;
}

void SlideShow::setTextAabb(uint32_t slide, const bbe::Rectangle& value)
{
	slides[slide].textAabb = value;
}

Slide& SlideShow::getLastSlide()
{
	Slide* candidate = &slides.last();
	while (candidate->childSlides.getLength() > 0)
	{
		candidate = &candidate->childSlides.last();
	}
	return *candidate;
}

void SlideShow::addSlidesToList(Slide* slide, bbe::List<Slide*> &list)
{
	list.add(slide);
	for (size_t i = 0; i < slide->childSlides.getLength(); i++)
	{
		addSlidesToList(&slide->childSlides[i], list);
	}
}

Slide& SlideShow::getSecondToLastSlide()
{
	//TODO this implementation is dumb and slow and dumb, fix plz

	bbe::List<Slide*> slides;
	for (size_t i = 0; i < this->slides.getLength(); i++)
	{
		addSlidesToList(&(this->slides[i]), slides);
	}
	return *slides[slides.getLength() - 2];
}
