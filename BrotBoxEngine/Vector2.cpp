#include "BBE/Vector2.h"

template<>
uint32_t bbe::hash(const Vector2i& t)
{
	return (t.x ^ t.y) + t.x * 7001 + t.y * 17;
}

template<>
uint32_t bbe::hash(const Vector2& t)
{
	return (((int32_t)t.x) ^ ((int32_t)t.y)) + t.x * 7001.f + t.y * 17.f;
}
