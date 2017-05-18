#pragma once


namespace bbe
{
	template<typename T>
	constexpr T nextMultiple(const T &multipleOf, const T &value)
	{
		return (value + multipleOf - 1) - ((value + multipleOf - 1) % multipleOf);
	}

	template<typename T>
	constexpr T abs(const T &val)
	{
		//UNTESTED
		if (val < 0) {
			return -val;
		}

		return val;
	}
}