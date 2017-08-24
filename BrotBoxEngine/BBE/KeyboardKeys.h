#pragma once


#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace Keys
	{
		const int LAST = GLFW_KEY_LAST;

		const int SPACE = GLFW_KEY_SPACE;
		const int APOSTROPHE = GLFW_KEY_APOSTROPHE;
		const int COMMA = GLFW_KEY_COMMA;
		const int MINUS = GLFW_KEY_MINUS;
		const int PERIOD = GLFW_KEY_PERIOD;
		const int SLASH = GLFW_KEY_SLASH;
		const int _0 = GLFW_KEY_0;
		const int _1 = GLFW_KEY_1;
		const int _2 = GLFW_KEY_2;
		const int _3 = GLFW_KEY_3;
		const int _4 = GLFW_KEY_4;
		const int _5 = GLFW_KEY_5;
		const int _6 = GLFW_KEY_6;
		const int _7 = GLFW_KEY_7;
		const int _8 = GLFW_KEY_8;
		const int _9 = GLFW_KEY_9;
		const int SEMICOLON = GLFW_KEY_SEMICOLON;
		const int EQUAL = GLFW_KEY_EQUAL;
		const int A = GLFW_KEY_A;
		const int B = GLFW_KEY_B;
		const int C = GLFW_KEY_C;
		const int D = GLFW_KEY_D;
		const int E = GLFW_KEY_E;
		const int F = GLFW_KEY_F;
		const int G = GLFW_KEY_G;
		const int H = GLFW_KEY_H;
		const int I = GLFW_KEY_I;
		const int J = GLFW_KEY_J;
		const int K = GLFW_KEY_K;
		const int L = GLFW_KEY_L;
		const int M = GLFW_KEY_M;
		const int N = GLFW_KEY_N;
		const int O = GLFW_KEY_O;
		const int P = GLFW_KEY_P;
		const int Q = GLFW_KEY_Q;
		const int R = GLFW_KEY_R;
		const int S = GLFW_KEY_S;
		const int T = GLFW_KEY_T;
		const int U = GLFW_KEY_U;
		const int V = GLFW_KEY_V;
		const int W = GLFW_KEY_W;
		const int X = GLFW_KEY_X;
		const int Y = GLFW_KEY_Y;
		const int Z = GLFW_KEY_Z;
		const int LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET;
		const int BACKSLASH = GLFW_KEY_BACKSLASH;
		const int RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET;
		const int GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT;
		const int WORLD_1 = GLFW_KEY_WORLD_1;
		const int WORLD_2 = GLFW_KEY_WORLD_2;
		const int ESCAPE = GLFW_KEY_ESCAPE;
		const int ENTER = GLFW_KEY_ENTER;
		const int TAB = GLFW_KEY_TAB;
		const int BACKSPACE = GLFW_KEY_BACKSPACE;
		const int INSERT = GLFW_KEY_INSERT;
		const int DELETE = GLFW_KEY_DELETE;
		const int RIGHT = GLFW_KEY_RIGHT;
		const int LEFT = GLFW_KEY_LEFT;
		const int DOWN = GLFW_KEY_DOWN;
		const int UP = GLFW_KEY_UP;
		const int PAGE_UP = GLFW_KEY_PAGE_UP;
		const int PAGE_DOWN = GLFW_KEY_PAGE_DOWN;
		const int HOME = GLFW_KEY_HOME;
		const int END = GLFW_KEY_END;
		const int CAPS_LOCK = GLFW_KEY_CAPS_LOCK;
		const int SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK;
		const int NUM_LOCK = GLFW_KEY_NUM_LOCK;
		const int PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN;
		const int PAUSE = GLFW_KEY_PAUSE;
		const int F1 = GLFW_KEY_F1;
		const int F2 = GLFW_KEY_F2;
		const int F3 = GLFW_KEY_F3;
		const int F4 = GLFW_KEY_F4;
		const int F5 = GLFW_KEY_F5;
		const int F6 = GLFW_KEY_F6;
		const int F7 = GLFW_KEY_F7;
		const int F8 = GLFW_KEY_F8;
		const int F9 = GLFW_KEY_F9;
		const int F10 = GLFW_KEY_F10;
		const int F11 = GLFW_KEY_F11;
		const int F12 = GLFW_KEY_F12;
		const int F13 = GLFW_KEY_F13;
		const int F14 = GLFW_KEY_F14;
		const int F15 = GLFW_KEY_F15;
		const int F16 = GLFW_KEY_F16;
		const int F17 = GLFW_KEY_F17;
		const int F18 = GLFW_KEY_F18;
		const int F19 = GLFW_KEY_F19;
		const int F20 = GLFW_KEY_F20;
		const int F21 = GLFW_KEY_F21;
		const int F22 = GLFW_KEY_F22;
		const int F23 = GLFW_KEY_F23;
		const int F24 = GLFW_KEY_F24;
		const int F25 = GLFW_KEY_F25;
		const int KP_0 = GLFW_KEY_KP_0;
		const int KP_1 = GLFW_KEY_KP_1;
		const int KP_2 = GLFW_KEY_KP_2;
		const int KP_3 = GLFW_KEY_KP_3;
		const int KP_4 = GLFW_KEY_KP_4;
		const int KP_5 = GLFW_KEY_KP_5;
		const int KP_6 = GLFW_KEY_KP_6;
		const int KP_7 = GLFW_KEY_KP_7;
		const int KP_8 = GLFW_KEY_KP_8;
		const int KP_9 = GLFW_KEY_KP_9;
		const int KP_DECIMAL = GLFW_KEY_KP_DECIMAL;
		const int KP_DIVIDE = GLFW_KEY_KP_DIVIDE;
		const int KP_MULTIPLY = GLFW_KEY_KP_MULTIPLY;
		const int KP_SUBTRACT = GLFW_KEY_KP_SUBTRACT;
		const int KP_ADD = GLFW_KEY_KP_ADD;
		const int KP_ENTER = GLFW_KEY_KP_ENTER;
		const int KP_EQUAL = GLFW_KEY_KP_EQUAL;
		const int LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
		const int LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL;
		const int LEFT_ALT = GLFW_KEY_LEFT_ALT;
		const int LEFT_SUPER = GLFW_KEY_LEFT_SUPER;
		const int RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT;
		const int RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL;
		const int RIGHT_ALT = GLFW_KEY_RIGHT_ALT;
		const int RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER;
		const int MENU = GLFW_KEY_MENU;
	}
	

	bbe::String keyCodeToString(int keyCode);
	bool isKeyCodeValid(int keyCode);
}
