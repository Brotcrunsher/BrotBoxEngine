#include "PngTokenizer.h"

void PngTokenizer::loadImage(const bbe::String& path)
{
	Token t;
	t.images.add(bbe::Image(path));

	tokens.add(t);
}
