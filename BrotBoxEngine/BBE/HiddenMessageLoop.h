#pragma once
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wtsapi32.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include "BBE/String.h"

#pragma comment(lib, "Wtsapi32.lib")

namespace bbe
{
	class HiddenMessageLoop
	{
	public:
		HiddenMessageLoop(const bbe::String& className);
		~HiddenMessageLoop();

		HiddenMessageLoop(const HiddenMessageLoop&) = delete;
		HiddenMessageLoop(HiddenMessageLoop&&) = delete;
		HiddenMessageLoop& operator=(const HiddenMessageLoop&) = delete;
		HiddenMessageLoop& operator=(HiddenMessageLoop&&) = delete;

		void start(const bbe::String& className);
		void stop();

	protected:
		virtual void messageLoopStart(HWND hwnd) {}
		virtual void messageLoopStop (HWND hwnd) {}
		virtual LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	private:
		std::thread messageThread;  // Thread for handling the message loop
		std::atomic<bool> running = false;  // Flag to indicate if the thread is running
		HWND hwnd = NULL;  // Window handle for the hidden window
		std::mutex hwndMutex;

		static LRESULT CALLBACK WindowProcc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void messageLoop(const bbe::String& className);
	};
}
#endif
