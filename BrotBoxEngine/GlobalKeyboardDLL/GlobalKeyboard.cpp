#include "GlobalKeyboardDLL/GlobalKeyboard.h"
#include "GlobalKeyboardDLL/EmbeddedDlls.h"

#include "BBE/SimpleFile.h"

#include <bitset>
#include <Windows.h>
#undef DELETE


typedef BOOL(WINAPI* GetNextEvent)(int* code, WPARAM* wParam, LPARAM* lParam, DWORD* pid);

bbe::GlobalKeyboard::~GlobalKeyboard()
{
	uninit();
}

void bbe::GlobalKeyboard::init()
{
	if (hooked) return;
	hooked = true;

	if (!bbe::simpleFile::doesFileExist("GlobalKeyboard.dll"))
	{
		bbe::simpleFile::writeBinaryToFile("GlobalKeyboard.dll", GlobalKeyboardDLL);
	}

	hmod = LoadLibrary("GlobalKeyboard.dll");
	HOOKPROC proc = (HOOKPROC)GetProcAddress(hmod, "KeyboardProc");
	hook = SetWindowsHookEx(WH_KEYBOARD, proc, hmod, 0);
	getNextEvent = (void*)GetProcAddress(hmod, "GetNextEvent");
}

void bbe::GlobalKeyboard::uninit()
{
	if (!hooked) return;
	hooked = false;

	UnhookWindowsHookEx(hook);
	FreeLibrary(hmod);
}

bool bbe::GlobalKeyboard::isInit() const
{
	return hooked;
}

static bbe::Key vkToBbe(WPARAM vk)
{
	switch (vk)
	{
	case VK_BACK     : return bbe::Key::BACKSPACE;
	case VK_TAB      : return bbe::Key::TAB;
	case VK_RETURN   : return bbe::Key::ENTER;
	case VK_SHIFT    : return bbe::Key::LEFT_SHIFT;
	case VK_CONTROL  : return bbe::Key::LEFT_CONTROL;
	case VK_MENU     : return bbe::Key::MENU;
	case VK_PAUSE    : return bbe::Key::PAUSE;
	case VK_CAPITAL  : return bbe::Key::CAPS_LOCK;
	case VK_ESCAPE   : return bbe::Key::ESCAPE;
	case VK_SPACE    : return bbe::Key::SPACE;
	case VK_LEFT     : return bbe::Key::LEFT;
	case VK_UP       : return bbe::Key::UP;
	case VK_RIGHT    : return bbe::Key::RIGHT;
	case VK_DOWN     : return bbe::Key::DOWN;
	case VK_INSERT   : return bbe::Key::INSERT;
	case VK_DELETE   : return bbe::Key::DELETE;
	case VK_OEM_PERIOD: return bbe::Key::PERIOD;
	case 0x30        : return bbe::Key::_0;
	case 0x31        : return bbe::Key::_1;
	case 0x32        : return bbe::Key::_2;
	case 0x33        : return bbe::Key::_3;
	case 0x34        : return bbe::Key::_4;
	case 0x35        : return bbe::Key::_5;
	case 0x36        : return bbe::Key::_6;
	case 0x37        : return bbe::Key::_7;
	case 0x38        : return bbe::Key::_8;
	case 0x39        : return bbe::Key::_9;
	case 0x41        : return bbe::Key::A;
	case 0x42        : return bbe::Key::B;
	case 0x43        : return bbe::Key::C;
	case 0x44        : return bbe::Key::D;
	case 0x45        : return bbe::Key::E;
	case 0x46        : return bbe::Key::F;
	case 0x47        : return bbe::Key::G;
	case 0x48        : return bbe::Key::H;
	case 0x49        : return bbe::Key::I;
	case 0x4A        : return bbe::Key::J;
	case 0x4B        : return bbe::Key::K;
	case 0x4C        : return bbe::Key::L;
	case 0x4D        : return bbe::Key::M;
	case 0x4E        : return bbe::Key::N;
	case 0x4F        : return bbe::Key::O;
	case 0x50        : return bbe::Key::P;
	case 0x51        : return bbe::Key::Q;
	case 0x52        : return bbe::Key::R;
	case 0x53        : return bbe::Key::S;
	case 0x54        : return bbe::Key::T;
	case 0x55        : return bbe::Key::U;
	case 0x56        : return bbe::Key::V;
	case 0x57        : return bbe::Key::W;
	case 0x58        : return bbe::Key::X;
	case 0x59        : return bbe::Key::Y;
	case 0x5A        : return bbe::Key::Z;
	case VK_LWIN	 : return bbe::Key::MENU;
	case VK_RWIN	 : return bbe::Key::MENU;
	case VK_NUMPAD0	 : return bbe::Key::KP_0;
	case VK_NUMPAD1	 : return bbe::Key::KP_1;
	case VK_NUMPAD2	 : return bbe::Key::KP_2;
	case VK_NUMPAD3	 : return bbe::Key::KP_3;
	case VK_NUMPAD4	 : return bbe::Key::KP_4;
	case VK_NUMPAD5	 : return bbe::Key::KP_5;
	case VK_NUMPAD6	 : return bbe::Key::KP_6;
	case VK_NUMPAD7	 : return bbe::Key::KP_7;
	case VK_NUMPAD8	 : return bbe::Key::KP_8;
	case VK_NUMPAD9	 : return bbe::Key::KP_9;
	case VK_MULTIPLY : return bbe::Key::KP_MULTIPLY;
	case VK_ADD		 : return bbe::Key::KP_ADD;
	case VK_SUBTRACT : return bbe::Key::KP_SUBTRACT;
	case VK_DECIMAL	 : return bbe::Key::KP_DECIMAL;
	case VK_DIVIDE	 : return bbe::Key::KP_DIVIDE;
	case VK_F1		 : return bbe::Key::F1;
	case VK_F2		 : return bbe::Key::F2;
	case VK_F3		 : return bbe::Key::F3;
	case VK_F4		 : return bbe::Key::F4;
	case VK_F5		 : return bbe::Key::F5;
	case VK_F6		 : return bbe::Key::F6;
	case VK_F7		 : return bbe::Key::F7;
	case VK_F8		 : return bbe::Key::F8;
	case VK_F9		 : return bbe::Key::F9;
	case VK_F10		 : return bbe::Key::F10;
	case VK_F11		 : return bbe::Key::F11;
	case VK_F12		 : return bbe::Key::F12;
	case VK_F13		 : return bbe::Key::F13;
	case VK_F14		 : return bbe::Key::F14;
	case VK_F15		 : return bbe::Key::F15;
	case VK_F16		 : return bbe::Key::F16;
	case VK_F17		 : return bbe::Key::F17;
	case VK_F18		 : return bbe::Key::F18;
	case VK_F19		 : return bbe::Key::F19;
	case VK_F20		 : return bbe::Key::F20;
	case VK_F21		 : return bbe::Key::F21;
	case VK_F22		 : return bbe::Key::F22;
	case VK_F23		 : return bbe::Key::F23;
	case VK_F24		 : return bbe::Key::F24;
	case VK_NUMLOCK	 : return bbe::Key::NUM_LOCK;
	case VK_SCROLL	 : return bbe::Key::SCROLL_LOCK;
	case VK_LSHIFT	 : return bbe::Key::LEFT_SHIFT;
	case VK_RSHIFT	 : return bbe::Key::RIGHT_SHIFT;
	case VK_LCONTROL : return bbe::Key::LEFT_CONTROL;
	case VK_RCONTROL : return bbe::Key::RIGHT_CONTROL;
	case VK_LMENU	 : return bbe::Key::MENU;
	case VK_RMENU	 : return bbe::Key::MENU;

	}
	return bbe::Key::INVALID;
}

void bbe::GlobalKeyboard::update()
{
	if (!hooked) throw bbe::IllegalArgumentException();
	bbe::Keyboard::update();

	int code = 0;
	WPARAM wParam = 0;
	LPARAM lParam = 0;
	DWORD pid = 0;
	while (GetNextEvent(getNextEvent)(&code, &wParam, &lParam, &pid))
	{
		bbe::Key k = vkToBbe(wParam);
		if (k != bbe::Key::INVALID)
		{
			bool pressed = 0 == ((lParam >> 31) & 1);
			if (pressed) bbe::Keyboard::INTERNAL_press(k);
			else         bbe::Keyboard::INTERNAL_release(k);
		}
	}
}
