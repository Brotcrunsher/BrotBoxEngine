#include "BBE/TrayIcon.h"
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>

static NOTIFYICONDATA notifyIconData = {};
static HWND Hwnd = 0;
static HMENU Hmenu = 0;
static HICON hIcon = 0;
static char szClassName[] = "MyWindowClassName";
static const char* myTooltip = nullptr;
static bbe::Game *myGame = nullptr;
static bbe::List<std::function<void()>> popupCallbacks;

#define WM_SYSICON        (WM_USER + 1)
#define CALLBACK_OFFSET 1000

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)                  /* handle the messages */
	{
	case WM_SYSICON:
	{
		if (lParam == WM_RBUTTONDOWN)
		{
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns

			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);



			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			if (clicked >= CALLBACK_OFFSET)
			{
				const size_t id = clicked - CALLBACK_OFFSET;
				popupCallbacks[id]();
			}
		}
		else if (lParam == WM_LBUTTONDBLCLK)
		{
			myGame->showWindow();
		}
		break;
	}

	case WM_DESTROY:

		PostQuitMessage(0);
		break;

	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void bbe::TrayIcon::init(bbe::Game* game, const char* tooltip, HICON icon)
{
	myGame = game;
	myTooltip = tooltip;

	WNDCLASSEX wincl = {};        /* Data structure for the windowclass */
	wincl.hInstance = GetModuleHandle(0);
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
	wincl.cbSize = sizeof(WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	//wincl.hIcon = hIcon;
	//wincl.hIconSm = hIcon;
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;                 /* No menu */
	wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                      /* structure or the window instance */
	wincl.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
	RegisterClassEx(&wincl);


	/* The class is registered, let's create the program*/
	Hwnd = CreateWindowEx(
		0,                   /* Extended possibilites for variation */
		szClassName,         /* Classname */
		szClassName,       /* Title Text */
		WS_OVERLAPPEDWINDOW, /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		544,                 /* The programs width */
		375,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /* No menu */
		GetModuleHandle(0),       /* Program Instance handler */
		NULL                 /* No Window Creation data */
	);

	setIcon(icon);
}

void bbe::TrayIcon::addPopupItem(const char* title, std::function<void()> callback)
{
	Hmenu = CreatePopupMenu();
	AppendMenu(Hmenu, MF_STRING, popupCallbacks.getLength() + CALLBACK_OFFSET, title);
	popupCallbacks.add(callback);
}

bool bbe::TrayIcon::isVisible()
{
	// Quite Hacky! Let me explain...
	// Switching a tray icon is an expensive operation, even when
	// the tray icon isn't even visible. So this function tries to
	// find out if the tray icon is visible or not. If not, we can
	// early out of the tray switching function (see
	// setCurrentTrayIcon). The only way to figure this out that I
	// found was asking for the position of the tray icon. When it's
	// not visible, the left attribute of the Rect returns something
	// around 300.
	// However! Getting the position of the tray icon is itself
	// (maybe surprisingly) an expensive opteration. So this function
	// only checks every second. If this second hasn't passed, then
	// it simply returns false. In a worse case this leads to a very
	// small (1 second) delay before the animation starts.
	// Honestly - this is dumb! But oh well. It kinda works.
	static bbe::TimePoint autoFalseUntil;

	if (!autoFalseUntil.hasPassed())
	{
		return false;
	}

	NOTIFYICONIDENTIFIER identifier = {};
	identifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	identifier.hWnd = Hwnd;
	identifier.uID = 0;
	memset(&identifier.guidItem, 0, sizeof(identifier.guidItem));
	RECT rect;
	Shell_NotifyIconGetRect(&identifier, &rect);

	bool retVal = rect.left > 500;
	if (!retVal) autoFalseUntil = bbe::TimePoint().plusSeconds(1);
	return retVal;
}

void bbe::TrayIcon::setIcon(HICON icon)
{
	static bool firstCall = true;
	static HICON previousHIcon = nullptr;
	if (previousHIcon == icon)
	{
		return;
	}
	previousHIcon = icon;

	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = Hwnd;
	notifyIconData.uID = 0;
	notifyIconData.uFlags = NIF_ICON | (firstCall ? (NIF_MESSAGE | NIF_TIP) : 0);
	notifyIconData.hIcon = icon;
	if (firstCall)
	{
		notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
		strcpy_s(notifyIconData.szTip, sizeof(notifyIconData.szTip), myTooltip);
	}

	Shell_NotifyIcon(firstCall ? NIM_ADD : NIM_MODIFY, &notifyIconData);
	firstCall = false;
}

#endif
