
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wtsapi32.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "Wtsapi32.lib")

namespace bbe
{
	class SessionLockMonitor
	{
	public:
		SessionLockMonitor();
		~SessionLockMonitor();

		void start();
		void stop();
		bool isScreenLocked() const;

	private:
		std::thread messageThread;  // Thread for handling the message loop
		std::atomic<bool> running = false;  // Flag to indicate if monitoring is running
		HWND hwnd = NULL;  // Window handle for the hidden window
		std::mutex hwndMutex;
		std::atomic<bool> isLocked = false;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void messageLoop();
	};
}
#endif
