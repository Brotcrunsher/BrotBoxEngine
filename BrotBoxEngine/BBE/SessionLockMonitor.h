
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "BBE/HiddenMessageLoop.h"

namespace bbe
{
	class SessionLockMonitor : public HiddenMessageLoop
	{
	public:
		SessionLockMonitor();

		bool isScreenLocked() const;

	protected:
		virtual void messageLoopStart(HWND hwnd) override;
		virtual void messageLoopStop(HWND hwnd) override;
		virtual LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	private:
		std::atomic<bool> isLocked = false;
	};
}
#endif
