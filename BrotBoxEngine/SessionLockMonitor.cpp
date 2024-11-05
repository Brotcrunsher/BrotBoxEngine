#ifdef WIN32
#include "BBE/SessionLockMonitor.h"
#include "BBE/SimpleThread.h"

bbe::SessionLockMonitor::SessionLockMonitor()
{
	start();
}

bbe::SessionLockMonitor::~SessionLockMonitor()
{
	stop();
}

void bbe::SessionLockMonitor::start()
{
	if (!running)
	{
		running = true;
		messageThread = std::thread(&SessionLockMonitor::messageLoop, this);
		bbe::simpleThread::setName(messageThread, "BBE SessionLockMonitor");
	}
}

void bbe::SessionLockMonitor::stop()
{
	if (running)
	{
		running = false;  // Signal the thread to stop
		{
			std::lock_guard<std::mutex> _(hwndMutex);
			PostMessage(hwnd, WM_QUIT, 0, 0);  // Quit the message loop
		}
		if (messageThread.joinable()) {
			messageThread.join();  // Wait for the thread to finish
		}
	}
}

bool bbe::SessionLockMonitor::isScreenLocked() const
{
	return isLocked;
}

LRESULT bbe::SessionLockMonitor::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SessionLockMonitor* monitor = reinterpret_cast<SessionLockMonitor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (uMsg) {
	case WM_WTSSESSION_CHANGE:
		if (wParam == WTS_SESSION_LOCK) {
			monitor->isLocked = true;
		}
		else if (wParam == WTS_SESSION_UNLOCK) {
			monitor->isLocked = false;
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

void bbe::SessionLockMonitor::messageLoop()
{
	const char CLASS_NAME[] = "Hidden Window Class";
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	{
		std::lock_guard<std::mutex> _(hwndMutex);
		hwnd = CreateWindowEx(
			0, CLASS_NAME, "Hidden Session Lock Monitor", 0,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

		if (hwnd == NULL) {
			std::cerr << "Failed to create window!" << std::endl;
			return;
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		// Register for session change notifications
		WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
	}

	// Message loop
	MSG msg;
	while (running) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Unregister session notifications and clean up
	{
		std::lock_guard<std::mutex> _(hwndMutex);
		WTSUnRegisterSessionNotification(hwnd);
		DestroyWindow(hwnd);
	}
}
#endif
