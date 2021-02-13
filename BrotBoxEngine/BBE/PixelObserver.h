#pragma once

#include "../BBE/Color.h"
#include <cstdint>

namespace bbe
{
	namespace INTERNAL
	{
		struct PixelObserverPImpl;
	}

	class PixelObserver
	{
	private:
		INTERNAL::PixelObserverPImpl* m_pimpl = nullptr; // We don't want the gory platform details leak around.

	public:
		PixelObserver();
		~PixelObserver();

		// Making our live a little simpler. Copying or moving a PixelObserver does not make much sense right now anyway.
		PixelObserver(const PixelObserver& )            = delete;
		PixelObserver(      PixelObserver&&)            = delete;
		PixelObserver& operator=(const PixelObserver& ) = delete;
		PixelObserver& operator=(      PixelObserver&&) = delete;

		bbe::Color getColor(int32_t x, int32_t y) const;
		bbe::Color getColor(const bbe::Vector2 &pos) const;
	};
}
