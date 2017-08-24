#include "stdafx.h"
#include "BBE/KeyboardKeys.h"
#include "BBE/Exceptions.h"

bbe::String bbe::keyCodeToString(int keyCode)
{
	switch (keyCode)
	{
		case Keys::SPACE         :
			return bbe::String(L"SPACE");
		case Keys::APOSTROPHE    :
			return bbe::String(L"APOSTROPHE");
		case Keys::COMMA         :
			return bbe::String(L"COMMA");
		case Keys::MINUS         :
			return bbe::String(L"MINUS");
		case Keys::PERIOD        :
			return bbe::String(L"PERIOD");
		case Keys::SLASH         :
			return bbe::String(L"SLASH");
		case Keys::_0            :
			return bbe::String(L"0");
		case Keys::_1            :
			return bbe::String(L"1");
		case Keys::_2            :
			return bbe::String(L"2");
		case Keys::_3            :
			return bbe::String(L"3");
		case Keys::_4            :
			return bbe::String(L"4");
		case Keys::_5            :
			return bbe::String(L"5");
		case Keys::_6            :
			return bbe::String(L"6");
		case Keys::_7            :
			return bbe::String(L"7");
		case Keys::_8            :
			return bbe::String(L"8");
		case Keys::_9            :
			return bbe::String(L"9");
		case Keys::SEMICOLON     :
			return bbe::String(L"SEMICOLON");
		case Keys::EQUAL         :
			return bbe::String(L"EQUAL");
		case Keys::A             :
			return bbe::String(L"A");
		case Keys::B             :
			return bbe::String(L"B");
		case Keys::C             :
			return bbe::String(L"C");
		case Keys::D             :
			return bbe::String(L"D");
		case Keys::E             :
			return bbe::String(L"E");
		case Keys::F             :
			return bbe::String(L"F");
		case Keys::G             :
			return bbe::String(L"G");
		case Keys::H             :
			return bbe::String(L"H");
		case Keys::I             :
			return bbe::String(L"I");
		case Keys::J             :
			return bbe::String(L"J");
		case Keys::K             :
			return bbe::String(L"K");
		case Keys::L             :
			return bbe::String(L"L");
		case Keys::M             :
			return bbe::String(L"M");
		case Keys::N             :
			return bbe::String(L"N");
		case Keys::O             :
			return bbe::String(L"O");
		case Keys::P             :
			return bbe::String(L"P");
		case Keys::Q             :
			return bbe::String(L"Q");
		case Keys::R             :
			return bbe::String(L"R");
		case Keys::S             :
			return bbe::String(L"S");
		case Keys::T             :
			return bbe::String(L"T");
		case Keys::U             :
			return bbe::String(L"U");
		case Keys::V             :
			return bbe::String(L"V");
		case Keys::W             :
			return bbe::String(L"W");
		case Keys::X             :
			return bbe::String(L"X");
		case Keys::Y             :
			return bbe::String(L"Y");
		case Keys::Z             :
			return bbe::String(L"Z");
		case Keys::LEFT_BRACKET  :
			return bbe::String(L"LEFT_BRACKET");
		case Keys::BACKSLASH     :
			return bbe::String(L"BACKSLASH");
		case Keys::RIGHT_BRACKET :
			return bbe::String(L"RIGHT_BRACKET");
		case Keys::GRAVE_ACCENT  :
			return bbe::String(L"GRAVE_ACCENT");
		case Keys::WORLD_1       :
			return bbe::String(L"WORLD_1");
		case Keys::WORLD_2       :
			return bbe::String(L"WORLD_2");
		case Keys::ESCAPE        :
			return bbe::String(L"ESCAPE");
		case Keys::ENTER         :
			return bbe::String(L"ENTER");
		case Keys::TAB           :
			return bbe::String(L"TAB");
		case Keys::BACKSPACE     :
			return bbe::String(L"BACKSPACE");
		case Keys::INSERT        :
			return bbe::String(L"INSERT");
		case Keys::DELETE        :
			return bbe::String(L"DELETE");
		case Keys::RIGHT         :
			return bbe::String(L"RIGHT");
		case Keys::LEFT          :
			return bbe::String(L"LEFT");
		case Keys::DOWN          :
			return bbe::String(L"DOWN");
		case Keys::UP            :
			return bbe::String(L"UP");
		case Keys::PAGE_UP       :
			return bbe::String(L"PAGE_UP");
		case Keys::PAGE_DOWN     :
			return bbe::String(L"PAGE_DOWN");
		case Keys::HOME          :
			return bbe::String(L"HOME");
		case Keys::END           :
			return bbe::String(L"END");
		case Keys::CAPS_LOCK     :
			return bbe::String(L"CAPS_LOCK");
		case Keys::SCROLL_LOCK   :
			return bbe::String(L"SCROLL_LOCK");
		case Keys::NUM_LOCK      :
			return bbe::String(L"NUM_LOCK");
		case Keys::PRINT_SCREEN  :
			return bbe::String(L"PRINT_SCREEN");
		case Keys::PAUSE         :
			return bbe::String(L"PAUSE");
		case Keys::F1            :
			return bbe::String(L"F1");
		case Keys::F2            :
			return bbe::String(L"F2");
		case Keys::F3            :
			return bbe::String(L"F3");
		case Keys::F4            :
			return bbe::String(L"F4");
		case Keys::F5            :
			return bbe::String(L"F5");
		case Keys::F6            :
			return bbe::String(L"F6");
		case Keys::F7            :
			return bbe::String(L"F7");
		case Keys::F8            :
			return bbe::String(L"F8");
		case Keys::F9            :
			return bbe::String(L"F9");
		case Keys::F10           :
			return bbe::String(L"F10");
		case Keys::F11           :
			return bbe::String(L"F11");
		case Keys::F12           :
			return bbe::String(L"F12");
		case Keys::F13           :
			return bbe::String(L"F13");
		case Keys::F14           :
			return bbe::String(L"F14");
		case Keys::F15           :
			return bbe::String(L"F15");
		case Keys::F16           :
			return bbe::String(L"F16");
		case Keys::F17           :
			return bbe::String(L"F17");
		case Keys::F18           :
			return bbe::String(L"F18");
		case Keys::F19           :
			return bbe::String(L"F19");
		case Keys::F20           :
			return bbe::String(L"F20");
		case Keys::F21           :
			return bbe::String(L"F21");
		case Keys::F22           :
			return bbe::String(L"F22");
		case Keys::F23           :
			return bbe::String(L"F23");
		case Keys::F24           :
			return bbe::String(L"F24");
		case Keys::F25           :
			return bbe::String(L"F25");
		case Keys::KP_0          :
			return bbe::String(L"KP_0");
		case Keys::KP_1          :
			return bbe::String(L"KP_1");
		case Keys::KP_2          :
			return bbe::String(L"KP_2");
		case Keys::KP_3          :
			return bbe::String(L"KP_3");
		case Keys::KP_4          :
			return bbe::String(L"KP_4");
		case Keys::KP_5          :
			return bbe::String(L"KP_5");
		case Keys::KP_6          :
			return bbe::String(L"KP_6");
		case Keys::KP_7          :
			return bbe::String(L"KP_7");
		case Keys::KP_8          :
			return bbe::String(L"KP_8");
		case Keys::KP_9          :
			return bbe::String(L"KP_9");
		case Keys::KP_DECIMAL    :
			return bbe::String(L"KP_DECIMAL");
		case Keys::KP_DIVIDE     :
			return bbe::String(L"KP_DIVIDE");
		case Keys::KP_MULTIPLY   :
			return bbe::String(L"KP_MULTIPLY");
		case Keys::KP_SUBTRACT   :
			return bbe::String(L"KP_SUBTRACT");
		case Keys::KP_ADD        :
			return bbe::String(L"KP_ADD");
		case Keys::KP_ENTER      :
			return bbe::String(L"KP_ENTER");
		case Keys::KP_EQUAL      :
			return bbe::String(L"KP_EQUAL");
		case Keys::LEFT_SHIFT    :
			return bbe::String(L"LEFT_SHIFT");
		case Keys::LEFT_CONTROL  :
			return bbe::String(L"LEFT_CONTROL");
		case Keys::LEFT_ALT      :
			return bbe::String(L"LEFT_ALT");
		case Keys::LEFT_SUPER    :
			return bbe::String(L"LEFT_SUPER");
		case Keys::RIGHT_SHIFT   :
			return bbe::String(L"RIGHT_SHIFT");
		case Keys::RIGHT_CONTROL :
			return bbe::String(L"RIGHT_CONTROL");
		case Keys::RIGHT_ALT     :
			return bbe::String(L"RIGHT_ALT");
		case Keys::RIGHT_SUPER   :
			return bbe::String(L"RIGHT_SUPER");
		case Keys::MENU          :
			return bbe::String(L"MENU");
	}
	
	throw NoSuchKeycodeException();

}

bool bbe::isKeyCodeValid(int keyCode)
{
	switch (keyCode)
	{
		case Keys::SPACE         :
		case Keys::APOSTROPHE    :
		case Keys::COMMA         :
		case Keys::MINUS         :
		case Keys::PERIOD        :
		case Keys::SLASH         :
		case Keys::_0            :
		case Keys::_1            :
		case Keys::_2            :
		case Keys::_3            :
		case Keys::_4            :
		case Keys::_5            :
		case Keys::_6            :
		case Keys::_7            :
		case Keys::_8            :
		case Keys::_9            :
		case Keys::SEMICOLON     :
		case Keys::EQUAL         :
		case Keys::A             :
		case Keys::B             :
		case Keys::C             :
		case Keys::D             :
		case Keys::E             :
		case Keys::F             :
		case Keys::G             :
		case Keys::H             :
		case Keys::I             :
		case Keys::J             :
		case Keys::K             :
		case Keys::L             :
		case Keys::M             :
		case Keys::N             :
		case Keys::O             :
		case Keys::P             :
		case Keys::Q             :
		case Keys::R             :
		case Keys::S             :
		case Keys::T             :
		case Keys::U             :
		case Keys::V             :
		case Keys::W             :
		case Keys::X             :
		case Keys::Y             :
		case Keys::Z             :
		case Keys::LEFT_BRACKET  :
		case Keys::BACKSLASH     :
		case Keys::RIGHT_BRACKET :
		case Keys::GRAVE_ACCENT  :
		case Keys::WORLD_1       :
		case Keys::WORLD_2       :
		case Keys::ESCAPE        :
		case Keys::ENTER         :
		case Keys::TAB           :
		case Keys::BACKSPACE     :
		case Keys::INSERT        :
		case Keys::DELETE        :
		case Keys::RIGHT         :
		case Keys::LEFT          :
		case Keys::DOWN          :
		case Keys::UP            :
		case Keys::PAGE_UP       :
		case Keys::PAGE_DOWN     :
		case Keys::HOME          :
		case Keys::END           :
		case Keys::CAPS_LOCK     :
		case Keys::SCROLL_LOCK   :
		case Keys::NUM_LOCK      :
		case Keys::PRINT_SCREEN  :
		case Keys::PAUSE         :
		case Keys::F1            :
		case Keys::F2            :
		case Keys::F3            :
		case Keys::F4            :
		case Keys::F5            :
		case Keys::F6            :
		case Keys::F7            :
		case Keys::F8            :
		case Keys::F9            :
		case Keys::F10           :
		case Keys::F11           :
		case Keys::F12           :
		case Keys::F13           :
		case Keys::F14           :
		case Keys::F15           :
		case Keys::F16           :
		case Keys::F17           :
		case Keys::F18           :
		case Keys::F19           :
		case Keys::F20           :
		case Keys::F21           :
		case Keys::F22           :
		case Keys::F23           :
		case Keys::F24           :
		case Keys::F25           :
		case Keys::KP_0          :
		case Keys::KP_1          :
		case Keys::KP_2          :
		case Keys::KP_3          :
		case Keys::KP_4          :
		case Keys::KP_5          :
		case Keys::KP_6          :
		case Keys::KP_7          :
		case Keys::KP_8          :
		case Keys::KP_9          :
		case Keys::KP_DECIMAL    :
		case Keys::KP_DIVIDE     :
		case Keys::KP_MULTIPLY   :
		case Keys::KP_SUBTRACT   :
		case Keys::KP_ADD        :
		case Keys::KP_ENTER      :
		case Keys::KP_EQUAL      :
		case Keys::LEFT_SHIFT    :
		case Keys::LEFT_CONTROL  :
		case Keys::LEFT_ALT      :
		case Keys::LEFT_SUPER    :
		case Keys::RIGHT_SHIFT   :
		case Keys::RIGHT_CONTROL :
		case Keys::RIGHT_ALT     :
		case Keys::RIGHT_SUPER   :
		case Keys::MENU          :
			return true;
	}
	return false;
}
