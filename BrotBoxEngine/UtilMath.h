#pragma once


namespace bbe
{
	template<typename T>
	constexpr T nextMultiple(T multipleOf, T value)
	{
		return (value + multipleOf - 1) - ((value + multipleOf - 1) % multipleOf);
	}
}