#ifdef WIN32
#include "BBE/HiddenMessageLoop.h"
#include "BBE/SimpleThread.h"

bbe::HiddenMessageLoop::HiddenMessageLoop(const bbe::String& className)
{
	start(className);
}

bbe::HiddenMessageLoop::~HiddenMessageLoop()
{
	stop();
}

void bbe::HiddenMessageLoop::start(const bbe::String& className)
{
	if (!running)
	{
		running = true;
		messageThread = std::thread(&HiddenMessageLoop::messageLoop, this, className);
		bbe::simpleThread::setName(messageThread, "BBE SessionLockMonitor");
	}
}

void bbe::HiddenMessageLoop::stop()
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

LRESULT bbe::HiddenMessageLoop::WindowProcc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HiddenMessageLoop* loop = reinterpret_cast<HiddenMessageLoop*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (loop)
	{
		return loop->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void bbe::HiddenMessageLoop::messageLoop(const bbe::String& className)
{
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProcc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = className.getRaw();

	RegisterClass(&wc);

	{
		std::lock_guard<std::mutex> _(hwndMutex);
		hwnd = CreateWindowEx(
			0, className.getRaw(), "Hidden Session Lock Monitor", 0,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

		if (hwnd == NULL) {
			std::cerr << "Failed to create window!" << std::endl;
			return;
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		messageLoopStart(hwnd);
	}

	// Message loop
	MSG msg;
	while (running) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	{
		std::lock_guard<std::mutex> _(hwndMutex);
		messageLoopStart(hwnd);
	}
}
#endif
