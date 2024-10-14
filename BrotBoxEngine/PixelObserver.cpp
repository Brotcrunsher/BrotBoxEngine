#include "BBE/PixelObserver.h"
#include "BBE/Vector2.h"
#include "BBE/Error.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace bbe
{
	namespace INTERNAL
	{
		struct PixelObserverPImpl
		{
#ifdef _WIN32
			HDC m_hdc;
#endif
		};
	}
}


bbe::PixelObserver::PixelObserver() :
	m_pimpl(new bbe::INTERNAL::PixelObserverPImpl())
{
#ifdef _WIN32
	m_pimpl->m_hdc = GetDC(NULL);
#endif
}

bbe::PixelObserver::~PixelObserver()
{
#ifdef _WIN32
	if (m_pimpl != nullptr)
	{
		ReleaseDC(NULL, m_pimpl->m_hdc);
	}
#endif
	delete m_pimpl;
	m_pimpl = nullptr;
}

bbe::Color bbe::PixelObserver::getColor(int32_t x, int32_t y) const
{
#ifdef _WIN32
	const COLORREF pixel = GetPixel(m_pimpl->m_hdc, x, y);
	const DWORD r = GetRValue(pixel);
	const DWORD g = GetGValue(pixel);
	const DWORD b = GetBValue(pixel);
	
	return bbe::Color(
		r / 255.f,
		g / 255.f,
		b / 255.f
	);
#else
	bbe::Crash(bbe::Error::NotImplemented);
#endif
}

bbe::Color bbe::PixelObserver::getColor(const bbe::Vector2& pos) const
{
	return getColor((int32_t)pos.x, (int32_t)pos.y);
}
