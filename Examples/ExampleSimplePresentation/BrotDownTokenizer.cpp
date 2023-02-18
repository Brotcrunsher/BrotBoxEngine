#include "BrotDownTokenizer.h"

extern bbe::List<bbe::Font> fonts;

BrotDownTokenizer::BrotDownTokenizer(const bbe::String& parentPath)
	: parentPath(parentPath)
{

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
		if (newLine && text[i] == '!')
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

			for (int32_t repeat = 0; repeat < repeats; repeat++)
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
					const bbe::Vector2 endPos = currentToken.renderObjects[indices.last()].getPos(0);
					const bbe::Vector2 diffPos = endPos - startPos;
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
						ro.fillColor = bbe::Color(0, 0, 0, 0);
						ro.outlineColor = bbe::Color(0, 0, 0, 0);
						ro.textColor = bbe::Color(200.f / 255.f, 200.f / 255.f, 200.f / 255.f);
					}
					else if (rt == RenderType::IMAGE)
					{
						ro.image.reset(new bbe::Image((parentPath + ro.text).getRaw()));
					}

					currentToken.renderObjects.add(ro);
					commandRecognized = true;
				}

				if (!commandRecognized)
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
					stopLine += font.getDimensions(noTilde[posAccess]).as<float>();
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
