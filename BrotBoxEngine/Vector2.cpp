#include "BBE/Vector2.h"

template<>
uint32_t bbe::hash(const Vector2i& t)
{
	return (t.x ^ t.y) + t.x * 7001 + t.y * 17;
}

template<>
uint32_t bbe::hash(const Vector2& t)
{
	return uint32_t((((int32_t)t.x) ^ ((int32_t)t.y)) + t.x * 7001.f + t.y * 17.f);
}

void bbe::LineIterator::init(const bbe::Vector2i& a, const bbe::Vector2i& b)
{
	this->a = a;
	this->b = b;
	diff = (b - a).abs();
	step.x = (a.x < b.x) ? 1 : -1;
	step.y = (a.y < b.y) ? 1 : -1;
	error = diff.x - diff.y;
}

bbe::LineIterator::LineIterator()
{
}

bbe::LineIterator::LineIterator(const bbe::Vector2i& a, const bbe::Vector2i& b)
{
	init(a, b);
}

bool bbe::LineIterator::hasNext() const
{
	return moreAvailable;
}

bbe::Vector2i bbe::LineIterator::next()
{
	moreAvailable &= a != b;
	const bbe::Vector2i retVal = a;
	
	const int32_t error2 = error * 2;
	if (error2 > -diff.y) {
		error -= diff.y;
		a.x += step.x;
	}
	if (error2 < diff.x) {
		error += diff.x;
		a.y += step.y;
	}
	
	return retVal;
}
