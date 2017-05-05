#pragma once

namespace bbe {
	template <typename T, int SIZE>
	class Array {
	private:
		T data[SIZE];
	public:
		T& operator[](size_t index) {
			return data[index];
		}
	};
}