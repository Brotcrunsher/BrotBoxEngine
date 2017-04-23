#pragma once


namespace bbe {
	template<typename T>
	T nextMultiple(T multipleOf, T value) {
		T multiple = value + multipleOf - 1;
		multiple -= (multiple % multipleOf);
		return multiple;
	}
}