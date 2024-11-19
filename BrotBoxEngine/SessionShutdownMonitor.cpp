#if 0
#ifdef WIN32
#include "BBE/SessionShutdownMonitor.h"
#include "BBE/SimpleThread.h"

static const char* windowName = "BBE: Session Shutdown Monitor";

bbe::SessionShutdownMonitor::SessionShutdownMonitor() :
	HiddenMessageLoop(windowName)
{
}

bbe::SessionShutdownMonitor::SessionShutdownMonitor(std::function<void()> callback) :
	HiddenMessageLoop(windowName),
	callback(callback)
{}

void bbe::SessionShutdownMonitor::setCallback(std::function<void()> callback)
{
	this->callback = callback;
}

LRESULT bbe::SessionShutdownMonitor::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_ENDSESSION:
		// This doesn't seem to be called - why?!
		execCallback();
		break;
	case WM_DESTROY:
		execCallback();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
void bbe::SessionShutdownMonitor::execCallback()
{
	if (callback)
	{
		callback();
		callback = nullptr;
	}
}
#endif
#endif
