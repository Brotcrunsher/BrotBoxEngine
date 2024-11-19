#ifdef WIN32
#include "BBE/SessionLockMonitor.h"
#include "BBE/SimpleThread.h"

bbe::SessionLockMonitor::SessionLockMonitor() : HiddenMessageLoop("BBE: Session Lock Monitor")
{}

void bbe::SessionLockMonitor::messageLoopStart(HWND hwnd)
{
	// Register for session change notifications
	WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
}

void bbe::SessionLockMonitor::messageLoopStop(HWND hwnd)
{
	// Unregister session notifications and clean up
	WTSUnRegisterSessionNotification(hwnd);
	DestroyWindow(hwnd);
}

LRESULT bbe::SessionLockMonitor::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_WTSSESSION_CHANGE:
		if (wParam == WTS_SESSION_LOCK) {
			isLocked = true;
		}
		else if (wParam == WTS_SESSION_UNLOCK) {
			isLocked = false;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
#endif

bool bbe::SessionLockMonitor::isScreenLocked() const
{
	return isLocked;
}
