
#if 0 //TODO: For some reason WM_ENDSESSION doesn't seem to be called.
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "BBE/HiddenMessageLoop.h"
#include <functional>

namespace bbe
{
	class SessionShutdownMonitor : public HiddenMessageLoop
	{
	public:
		SessionShutdownMonitor();
		SessionShutdownMonitor(std::function<void()> callback);

		void setCallback(std::function<void()> callback);

	protected:
		virtual LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	private:
		void execCallback();
		std::function<void()> callback = nullptr;
	};
}
#endif
#endif
