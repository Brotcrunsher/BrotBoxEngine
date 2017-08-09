#pragma once


namespace bbe
{
	class Vector2;

	class Vector3
	{
	public:
		float x;
		float y;
		float z;

		Vector3();
		Vector3(float x, float y, float z);
		Vector3(float x, Vector2 yz);
		Vector3(Vector2 xy, float z);
	};
}