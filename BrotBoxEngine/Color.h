#pragma once


namespace bbe
{
	class Color
	{
	public:
		float r;
		float g;
		float b;
		float a;

		Color();
		Color(float r, float g, float b);
		Color(float r, float g, float b, float a);

	};
}