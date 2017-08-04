#include "stdafx.h"
#include "KeyboardKeys.h"
#include "Exceptions.h"

bbe::String bbe::keyCodeToString(int keyCode)
{
	switch (keyCode)
	{
		case KEY_SPACE         :
			return bbe::String(L"KEY_SPACE");
		case KEY_APOSTROPHE    :
			return bbe::String("KEY_APOSTROPHE");
		case KEY_COMMA         :
			return bbe::String("KEY_COMMA");
		case KEY_MINUS         :
			return bbe::String("KEY_MINUS");
		case KEY_PERIOD        :
			return bbe::String("KEY_PERIOD");
		case KEY_SLASH         :
			return bbe::String("KEY_SLASH");
		case KEY_0             :
			return bbe::String("KEY_0");
		case KEY_1             :
			return bbe::String("KEY_1");
		case KEY_2             :
			return bbe::String("KEY_2");
		case KEY_3             :
			return bbe::String("KEY_3");
		case KEY_4             :
			return bbe::String("KEY_4");
		case KEY_5             :
			return bbe::String("KEY_5");
		case KEY_6             :
			return bbe::String("KEY_6");
		case KEY_7             :
			return bbe::String("KEY_7");
		case KEY_8             :
			return bbe::String("KEY_8");
		case KEY_9             :
			return bbe::String("KEY_9");
		case KEY_SEMICOLON     :
			return bbe::String("KEY_SEMICOLON");
		case KEY_EQUAL         :
			return bbe::String("KEY_EQUAL");
		case KEY_A             :
			return bbe::String("KEY_A");
		case KEY_B             :
			return bbe::String("KEY_B");
		case KEY_C             :
			return bbe::String("KEY_C");
		case KEY_D             :
			return bbe::String("KEY_D");
		case KEY_E             :
			return bbe::String("KEY_E");
		case KEY_F             :
			return bbe::String("KEY_F");
		case KEY_G             :
			return bbe::String("KEY_G");
		case KEY_H             :
			return bbe::String("KEY_H");
		case KEY_I             :
			return bbe::String("KEY_I");
		case KEY_J             :
			return bbe::String("KEY_J");
		case KEY_K             :
			return bbe::String("KEY_K");
		case KEY_L             :
			return bbe::String("KEY_L");
		case KEY_M             :
			return bbe::String("KEY_M");
		case KEY_N             :
			return bbe::String("KEY_N");
		case KEY_O             :
			return bbe::String("KEY_O");
		case KEY_P             :
			return bbe::String("KEY_P");
		case KEY_Q             :
			return bbe::String("KEY_Q");
		case KEY_R             :
			return bbe::String("KEY_R");
		case KEY_S             :
			return bbe::String("KEY_S");
		case KEY_T             :
			return bbe::String("KEY_T");
		case KEY_U             :
			return bbe::String("KEY_U");
		case KEY_V             :
			return bbe::String("KEY_V");
		case KEY_W             :
			return bbe::String("KEY_W");
		case KEY_X             :
			return bbe::String("KEY_X");
		case KEY_Y             :
			return bbe::String("KEY_Y");
		case KEY_Z             :
			return bbe::String("KEY_Z");
		case KEY_LEFT_BRACKET  :
			return bbe::String("KEY_LEFT_BRACKET");
		case KEY_BACKSLASH     :
			return bbe::String("KEY_BACKSLASH");
		case KEY_RIGHT_BRACKET :
			return bbe::String("KEY_RIGHT_BRACKET");
		case KEY_GRAVE_ACCENT  :
			return bbe::String("KEY_GRAVE_ACCENT");
		case KEY_WORLD_1       :
			return bbe::String("KEY_WORLD_1");
		case KEY_WORLD_2       :
			return bbe::String("KEY_WORLD_2");
		case KEY_ESCAPE        :
			return bbe::String("KEY_ESCAPE");
		case KEY_ENTER         :
			return bbe::String("KEY_ENTER");
		case KEY_TAB           :
			return bbe::String("KEY_TAB");
		case KEY_BACKSPACE     :
			return bbe::String("KEY_BACKSPACE");
		case KEY_INSERT        :
			return bbe::String("KEY_INSERT");
		case KEY_DELETE        :
			return bbe::String("KEY_DELETE");
		case KEY_RIGHT         :
			return bbe::String("KEY_RIGHT");
		case KEY_LEFT          :
			return bbe::String("KEY_LEFT");
		case KEY_DOWN          :
			return bbe::String("KEY_DOWN");
		case KEY_UP            :
			return bbe::String("KEY_UP");
		case KEY_PAGE_UP       :
			return bbe::String("KEY_PAGE_UP");
		case KEY_PAGE_DOWN     :
			return bbe::String("KEY_PAGE_DOWN");
		case KEY_HOME          :
			return bbe::String("KEY_HOME");
		case KEY_END           :
			return bbe::String("KEY_END");
		case KEY_CAPS_LOCK     :
			return bbe::String("KEY_CAPS_LOCK");
		case KEY_SCROLL_LOCK   :
			return bbe::String("KEY_SCROLL_LOCK");
		case KEY_NUM_LOCK      :
			return bbe::String("KEY_NUM_LOCK");
		case KEY_PRINT_SCREEN  :
			return bbe::String("KEY_PRINT_SCREEN");
		case KEY_PAUSE         :
			return bbe::String("KEY_PAUSE");
		case KEY_F1            :
			return bbe::String("KEY_F1");
		case KEY_F2            :
			return bbe::String("KEY_F2");
		case KEY_F3            :
			return bbe::String("KEY_F3");
		case KEY_F4            :
			return bbe::String("KEY_F4");
		case KEY_F5            :
			return bbe::String("KEY_F5");
		case KEY_F6            :
			return bbe::String("KEY_F6");
		case KEY_F7            :
			return bbe::String("KEY_F7");
		case KEY_F8            :
			return bbe::String("KEY_F8");
		case KEY_F9            :
			return bbe::String("KEY_F9");
		case KEY_F10           :
			return bbe::String("KEY_F10");
		case KEY_F11           :
			return bbe::String("KEY_F11");
		case KEY_F12           :
			return bbe::String("KEY_F12");
		case KEY_F13           :
			return bbe::String("KEY_F13");
		case KEY_F14           :
			return bbe::String("KEY_F14");
		case KEY_F15           :
			return bbe::String("KEY_F15");
		case KEY_F16           :
			return bbe::String("KEY_F16");
		case KEY_F17           :
			return bbe::String("KEY_F17");
		case KEY_F18           :
			return bbe::String("KEY_F18");
		case KEY_F19           :
			return bbe::String("KEY_F19");
		case KEY_F20           :
			return bbe::String("KEY_F20");
		case KEY_F21           :
			return bbe::String("KEY_F21");
		case KEY_F22           :
			return bbe::String("KEY_F22");
		case KEY_F23           :
			return bbe::String("KEY_F23");
		case KEY_F24           :
			return bbe::String("KEY_F24");
		case KEY_F25           :
			return bbe::String("KEY_F25");
		case KEY_KP_0          :
			return bbe::String("KEY_KP_0");
		case KEY_KP_1          :
			return bbe::String("KEY_KP_1");
		case KEY_KP_2          :
			return bbe::String("KEY_KP_2");
		case KEY_KP_3          :
			return bbe::String("KEY_KP_3");
		case KEY_KP_4          :
			return bbe::String("KEY_KP_4");
		case KEY_KP_5          :
			return bbe::String("KEY_KP_5");
		case KEY_KP_6          :
			return bbe::String("KEY_KP_6");
		case KEY_KP_7          :
			return bbe::String("KEY_KP_7");
		case KEY_KP_8          :
			return bbe::String("KEY_KP_8");
		case KEY_KP_9          :
			return bbe::String("KEY_KP_9");
		case KEY_KP_DECIMAL    :
			return bbe::String("KEY_KP_DECIMAL");
		case KEY_KP_DIVIDE     :
			return bbe::String("KEY_KP_DIVIDE");
		case KEY_KP_MULTIPLY   :
			return bbe::String("KEY_KP_MULTIPLY");
		case KEY_KP_SUBTRACT   :
			return bbe::String("KEY_KP_SUBTRACT");
		case KEY_KP_ADD        :
			return bbe::String("KEY_KP_ADD");
		case KEY_KP_ENTER      :
			return bbe::String("KEY_KP_ENTER");
		case KEY_KP_EQUAL      :
			return bbe::String("KEY_KP_EQUAL");
		case KEY_LEFT_SHIFT    :
			return bbe::String("KEY_LEFT_SHIFT");
		case KEY_LEFT_CONTROL  :
			return bbe::String("KEY_LEFT_CONTROL");
		case KEY_LEFT_ALT      :
			return bbe::String("KEY_LEFT_ALT");
		case KEY_LEFT_SUPER    :
			return bbe::String("KEY_LEFT_SUPER");
		case KEY_RIGHT_SHIFT   :
			return bbe::String("KEY_RIGHT_SHIFT");
		case KEY_RIGHT_CONTROL :
			return bbe::String("KEY_RIGHT_CONTROL");
		case KEY_RIGHT_ALT     :
			return bbe::String("KEY_RIGHT_ALT");
		case KEY_RIGHT_SUPER   :
			return bbe::String("KEY_RIGHT_SUPER");
		case KEY_MENU          :
			return bbe::String("KEY_MENU");
	}
	
	throw NoSuchKeycodeException();

}

bool bbe::isKeyCodeValid(int keyCode)
{
	switch (keyCode)
	{
		case KEY_SPACE         :
		case KEY_APOSTROPHE    :
		case KEY_COMMA         :
		case KEY_MINUS         :
		case KEY_PERIOD        :
		case KEY_SLASH         :
		case KEY_0             :
		case KEY_1             :
		case KEY_2             :
		case KEY_3             :
		case KEY_4             :
		case KEY_5             :
		case KEY_6             :
		case KEY_7             :
		case KEY_8             :
		case KEY_9             :
		case KEY_SEMICOLON     :
		case KEY_EQUAL         :
		case KEY_A             :
		case KEY_B             :
		case KEY_C             :
		case KEY_D             :
		case KEY_E             :
		case KEY_F             :
		case KEY_G             :
		case KEY_H             :
		case KEY_I             :
		case KEY_J             :
		case KEY_K             :
		case KEY_L             :
		case KEY_M             :
		case KEY_N             :
		case KEY_O             :
		case KEY_P             :
		case KEY_Q             :
		case KEY_R             :
		case KEY_S             :
		case KEY_T             :
		case KEY_U             :
		case KEY_V             :
		case KEY_W             :
		case KEY_X             :
		case KEY_Y             :
		case KEY_Z             :
		case KEY_LEFT_BRACKET  :
		case KEY_BACKSLASH     :
		case KEY_RIGHT_BRACKET :
		case KEY_GRAVE_ACCENT  :
		case KEY_WORLD_1       :
		case KEY_WORLD_2       :
		case KEY_ESCAPE        :
		case KEY_ENTER         :
		case KEY_TAB           :
		case KEY_BACKSPACE     :
		case KEY_INSERT        :
		case KEY_DELETE        :
		case KEY_RIGHT         :
		case KEY_LEFT          :
		case KEY_DOWN          :
		case KEY_UP            :
		case KEY_PAGE_UP       :
		case KEY_PAGE_DOWN     :
		case KEY_HOME          :
		case KEY_END           :
		case KEY_CAPS_LOCK     :
		case KEY_SCROLL_LOCK   :
		case KEY_NUM_LOCK      :
		case KEY_PRINT_SCREEN  :
		case KEY_PAUSE         :
		case KEY_F1            :
		case KEY_F2            :
		case KEY_F3            :
		case KEY_F4            :
		case KEY_F5            :
		case KEY_F6            :
		case KEY_F7            :
		case KEY_F8            :
		case KEY_F9            :
		case KEY_F10           :
		case KEY_F11           :
		case KEY_F12           :
		case KEY_F13           :
		case KEY_F14           :
		case KEY_F15           :
		case KEY_F16           :
		case KEY_F17           :
		case KEY_F18           :
		case KEY_F19           :
		case KEY_F20           :
		case KEY_F21           :
		case KEY_F22           :
		case KEY_F23           :
		case KEY_F24           :
		case KEY_F25           :
		case KEY_KP_0          :
		case KEY_KP_1          :
		case KEY_KP_2          :
		case KEY_KP_3          :
		case KEY_KP_4          :
		case KEY_KP_5          :
		case KEY_KP_6          :
		case KEY_KP_7          :
		case KEY_KP_8          :
		case KEY_KP_9          :
		case KEY_KP_DECIMAL    :
		case KEY_KP_DIVIDE     :
		case KEY_KP_MULTIPLY   :
		case KEY_KP_SUBTRACT   :
		case KEY_KP_ADD        :
		case KEY_KP_ENTER      :
		case KEY_KP_EQUAL      :
		case KEY_LEFT_SHIFT    :
		case KEY_LEFT_CONTROL  :
		case KEY_LEFT_ALT      :
		case KEY_LEFT_SUPER    :
		case KEY_RIGHT_SHIFT   :
		case KEY_RIGHT_CONTROL :
		case KEY_RIGHT_ALT     :
		case KEY_RIGHT_SUPER   :
		case KEY_MENU          :
			return true;
	}
	return false;
}
