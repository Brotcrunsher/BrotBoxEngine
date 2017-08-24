#include "stdafx.h"
#include "BBE/KeyboardKeys.h"
#include "BBE/Exceptions.h"

bbe::String bbe::keyCodeToString(bbe::Key key)
{
	switch (key)
	{
		case bbe::Key::SPACE         :
			return bbe::String(L"SPACE");
		case bbe::Key::APOSTROPHE    :
			return bbe::String(L"APOSTROPHE");
		case bbe::Key::COMMA         :
			return bbe::String(L"COMMA");
		case bbe::Key::MINUS         :
			return bbe::String(L"MINUS");
		case bbe::Key::PERIOD        :
			return bbe::String(L"PERIOD");
		case bbe::Key::SLASH         :
			return bbe::String(L"SLASH");
		case bbe::Key::_0            :
			return bbe::String(L"0");
		case bbe::Key::_1            :
			return bbe::String(L"1");
		case bbe::Key::_2            :
			return bbe::String(L"2");
		case bbe::Key::_3            :
			return bbe::String(L"3");
		case bbe::Key::_4            :
			return bbe::String(L"4");
		case bbe::Key::_5            :
			return bbe::String(L"5");
		case bbe::Key::_6            :
			return bbe::String(L"6");
		case bbe::Key::_7            :
			return bbe::String(L"7");
		case bbe::Key::_8            :
			return bbe::String(L"8");
		case bbe::Key::_9            :
			return bbe::String(L"9");
		case bbe::Key::SEMICOLON     :
			return bbe::String(L"SEMICOLON");
		case bbe::Key::EQUAL         :
			return bbe::String(L"EQUAL");
		case bbe::Key::A             :
			return bbe::String(L"A");
		case bbe::Key::B             :
			return bbe::String(L"B");
		case bbe::Key::C             :
			return bbe::String(L"C");
		case bbe::Key::D             :
			return bbe::String(L"D");
		case bbe::Key::E             :
			return bbe::String(L"E");
		case bbe::Key::F             :
			return bbe::String(L"F");
		case bbe::Key::G             :
			return bbe::String(L"G");
		case bbe::Key::H             :
			return bbe::String(L"H");
		case bbe::Key::I             :
			return bbe::String(L"I");
		case bbe::Key::J             :
			return bbe::String(L"J");
		case bbe::Key::K             :
			return bbe::String(L"K");
		case bbe::Key::L             :
			return bbe::String(L"L");
		case bbe::Key::M             :
			return bbe::String(L"M");
		case bbe::Key::N             :
			return bbe::String(L"N");
		case bbe::Key::O             :
			return bbe::String(L"O");
		case bbe::Key::P             :
			return bbe::String(L"P");
		case bbe::Key::Q             :
			return bbe::String(L"Q");
		case bbe::Key::R             :
			return bbe::String(L"R");
		case bbe::Key::S             :
			return bbe::String(L"S");
		case bbe::Key::T             :
			return bbe::String(L"T");
		case bbe::Key::U             :
			return bbe::String(L"U");
		case bbe::Key::V             :
			return bbe::String(L"V");
		case bbe::Key::W             :
			return bbe::String(L"W");
		case bbe::Key::X             :
			return bbe::String(L"X");
		case bbe::Key::Y             :
			return bbe::String(L"Y");
		case bbe::Key::Z             :
			return bbe::String(L"Z");
		case bbe::Key::LEFT_BRACKET  :
			return bbe::String(L"LEFT_BRACKET");
		case bbe::Key::BACKSLASH     :
			return bbe::String(L"BACKSLASH");
		case bbe::Key::RIGHT_BRACKET :
			return bbe::String(L"RIGHT_BRACKET");
		case bbe::Key::GRAVE_ACCENT  :
			return bbe::String(L"GRAVE_ACCENT");
		case bbe::Key::WORLD_1       :
			return bbe::String(L"WORLD_1");
		case bbe::Key::WORLD_2       :
			return bbe::String(L"WORLD_2");
		case bbe::Key::ESCAPE        :
			return bbe::String(L"ESCAPE");
		case bbe::Key::ENTER         :
			return bbe::String(L"ENTER");
		case bbe::Key::TAB           :
			return bbe::String(L"TAB");
		case bbe::Key::BACKSPACE     :
			return bbe::String(L"BACKSPACE");
		case bbe::Key::INSERT        :
			return bbe::String(L"INSERT");
		case bbe::Key::DELETE        :
			return bbe::String(L"DELETE");
		case bbe::Key::RIGHT         :
			return bbe::String(L"RIGHT");
		case bbe::Key::LEFT          :
			return bbe::String(L"LEFT");
		case bbe::Key::DOWN          :
			return bbe::String(L"DOWN");
		case bbe::Key::UP            :
			return bbe::String(L"UP");
		case bbe::Key::PAGE_UP       :
			return bbe::String(L"PAGE_UP");
		case bbe::Key::PAGE_DOWN     :
			return bbe::String(L"PAGE_DOWN");
		case bbe::Key::HOME          :
			return bbe::String(L"HOME");
		case bbe::Key::END           :
			return bbe::String(L"END");
		case bbe::Key::CAPS_LOCK     :
			return bbe::String(L"CAPS_LOCK");
		case bbe::Key::SCROLL_LOCK   :
			return bbe::String(L"SCROLL_LOCK");
		case bbe::Key::NUM_LOCK      :
			return bbe::String(L"NUM_LOCK");
		case bbe::Key::PRINT_SCREEN  :
			return bbe::String(L"PRINT_SCREEN");
		case bbe::Key::PAUSE         :
			return bbe::String(L"PAUSE");
		case bbe::Key::F1            :
			return bbe::String(L"F1");
		case bbe::Key::F2            :
			return bbe::String(L"F2");
		case bbe::Key::F3            :
			return bbe::String(L"F3");
		case bbe::Key::F4            :
			return bbe::String(L"F4");
		case bbe::Key::F5            :
			return bbe::String(L"F5");
		case bbe::Key::F6            :
			return bbe::String(L"F6");
		case bbe::Key::F7            :
			return bbe::String(L"F7");
		case bbe::Key::F8            :
			return bbe::String(L"F8");
		case bbe::Key::F9            :
			return bbe::String(L"F9");
		case bbe::Key::F10           :
			return bbe::String(L"F10");
		case bbe::Key::F11           :
			return bbe::String(L"F11");
		case bbe::Key::F12           :
			return bbe::String(L"F12");
		case bbe::Key::F13           :
			return bbe::String(L"F13");
		case bbe::Key::F14           :
			return bbe::String(L"F14");
		case bbe::Key::F15           :
			return bbe::String(L"F15");
		case bbe::Key::F16           :
			return bbe::String(L"F16");
		case bbe::Key::F17           :
			return bbe::String(L"F17");
		case bbe::Key::F18           :
			return bbe::String(L"F18");
		case bbe::Key::F19           :
			return bbe::String(L"F19");
		case bbe::Key::F20           :
			return bbe::String(L"F20");
		case bbe::Key::F21           :
			return bbe::String(L"F21");
		case bbe::Key::F22           :
			return bbe::String(L"F22");
		case bbe::Key::F23           :
			return bbe::String(L"F23");
		case bbe::Key::F24           :
			return bbe::String(L"F24");
		case bbe::Key::F25           :
			return bbe::String(L"F25");
		case bbe::Key::KP_0          :
			return bbe::String(L"KP_0");
		case bbe::Key::KP_1          :
			return bbe::String(L"KP_1");
		case bbe::Key::KP_2          :
			return bbe::String(L"KP_2");
		case bbe::Key::KP_3          :
			return bbe::String(L"KP_3");
		case bbe::Key::KP_4          :
			return bbe::String(L"KP_4");
		case bbe::Key::KP_5          :
			return bbe::String(L"KP_5");
		case bbe::Key::KP_6          :
			return bbe::String(L"KP_6");
		case bbe::Key::KP_7          :
			return bbe::String(L"KP_7");
		case bbe::Key::KP_8          :
			return bbe::String(L"KP_8");
		case bbe::Key::KP_9          :
			return bbe::String(L"KP_9");
		case bbe::Key::KP_DECIMAL    :
			return bbe::String(L"KP_DECIMAL");
		case bbe::Key::KP_DIVIDE     :
			return bbe::String(L"KP_DIVIDE");
		case bbe::Key::KP_MULTIPLY   :
			return bbe::String(L"KP_MULTIPLY");
		case bbe::Key::KP_SUBTRACT   :
			return bbe::String(L"KP_SUBTRACT");
		case bbe::Key::KP_ADD        :
			return bbe::String(L"KP_ADD");
		case bbe::Key::KP_ENTER      :
			return bbe::String(L"KP_ENTER");
		case bbe::Key::KP_EQUAL      :
			return bbe::String(L"KP_EQUAL");
		case bbe::Key::LEFT_SHIFT    :
			return bbe::String(L"LEFT_SHIFT");
		case bbe::Key::LEFT_CONTROL  :
			return bbe::String(L"LEFT_CONTROL");
		case bbe::Key::LEFT_ALT      :
			return bbe::String(L"LEFT_ALT");
		case bbe::Key::LEFT_SUPER    :
			return bbe::String(L"LEFT_SUPER");
		case bbe::Key::RIGHT_SHIFT   :
			return bbe::String(L"RIGHT_SHIFT");
		case bbe::Key::RIGHT_CONTROL :
			return bbe::String(L"RIGHT_CONTROL");
		case bbe::Key::RIGHT_ALT     :
			return bbe::String(L"RIGHT_ALT");
		case bbe::Key::RIGHT_SUPER   :
			return bbe::String(L"RIGHT_SUPER");
		case bbe::Key::MENU          :
			return bbe::String(L"MENU");
	}
	
	throw NoSuchKeycodeException();

}

bool bbe::isKeyCodeValid(bbe::Key key)
{
	switch (key)
	{
		case bbe::Key::SPACE         :
		case bbe::Key::APOSTROPHE    :
		case bbe::Key::COMMA         :
		case bbe::Key::MINUS         :
		case bbe::Key::PERIOD        :
		case bbe::Key::SLASH         :
		case bbe::Key::_0            :
		case bbe::Key::_1            :
		case bbe::Key::_2            :
		case bbe::Key::_3            :
		case bbe::Key::_4            :
		case bbe::Key::_5            :
		case bbe::Key::_6            :
		case bbe::Key::_7            :
		case bbe::Key::_8            :
		case bbe::Key::_9            :
		case bbe::Key::SEMICOLON     :
		case bbe::Key::EQUAL         :
		case bbe::Key::A             :
		case bbe::Key::B             :
		case bbe::Key::C             :
		case bbe::Key::D             :
		case bbe::Key::E             :
		case bbe::Key::F             :
		case bbe::Key::G             :
		case bbe::Key::H             :
		case bbe::Key::I             :
		case bbe::Key::J             :
		case bbe::Key::K             :
		case bbe::Key::L             :
		case bbe::Key::M             :
		case bbe::Key::N             :
		case bbe::Key::O             :
		case bbe::Key::P             :
		case bbe::Key::Q             :
		case bbe::Key::R             :
		case bbe::Key::S             :
		case bbe::Key::T             :
		case bbe::Key::U             :
		case bbe::Key::V             :
		case bbe::Key::W             :
		case bbe::Key::X             :
		case bbe::Key::Y             :
		case bbe::Key::Z             :
		case bbe::Key::LEFT_BRACKET  :
		case bbe::Key::BACKSLASH     :
		case bbe::Key::RIGHT_BRACKET :
		case bbe::Key::GRAVE_ACCENT  :
		case bbe::Key::WORLD_1       :
		case bbe::Key::WORLD_2       :
		case bbe::Key::ESCAPE        :
		case bbe::Key::ENTER         :
		case bbe::Key::TAB           :
		case bbe::Key::BACKSPACE     :
		case bbe::Key::INSERT        :
		case bbe::Key::DELETE        :
		case bbe::Key::RIGHT         :
		case bbe::Key::LEFT          :
		case bbe::Key::DOWN          :
		case bbe::Key::UP            :
		case bbe::Key::PAGE_UP       :
		case bbe::Key::PAGE_DOWN     :
		case bbe::Key::HOME          :
		case bbe::Key::END           :
		case bbe::Key::CAPS_LOCK     :
		case bbe::Key::SCROLL_LOCK   :
		case bbe::Key::NUM_LOCK      :
		case bbe::Key::PRINT_SCREEN  :
		case bbe::Key::PAUSE         :
		case bbe::Key::F1            :
		case bbe::Key::F2            :
		case bbe::Key::F3            :
		case bbe::Key::F4            :
		case bbe::Key::F5            :
		case bbe::Key::F6            :
		case bbe::Key::F7            :
		case bbe::Key::F8            :
		case bbe::Key::F9            :
		case bbe::Key::F10           :
		case bbe::Key::F11           :
		case bbe::Key::F12           :
		case bbe::Key::F13           :
		case bbe::Key::F14           :
		case bbe::Key::F15           :
		case bbe::Key::F16           :
		case bbe::Key::F17           :
		case bbe::Key::F18           :
		case bbe::Key::F19           :
		case bbe::Key::F20           :
		case bbe::Key::F21           :
		case bbe::Key::F22           :
		case bbe::Key::F23           :
		case bbe::Key::F24           :
		case bbe::Key::F25           :
		case bbe::Key::KP_0          :
		case bbe::Key::KP_1          :
		case bbe::Key::KP_2          :
		case bbe::Key::KP_3          :
		case bbe::Key::KP_4          :
		case bbe::Key::KP_5          :
		case bbe::Key::KP_6          :
		case bbe::Key::KP_7          :
		case bbe::Key::KP_8          :
		case bbe::Key::KP_9          :
		case bbe::Key::KP_DECIMAL    :
		case bbe::Key::KP_DIVIDE     :
		case bbe::Key::KP_MULTIPLY   :
		case bbe::Key::KP_SUBTRACT   :
		case bbe::Key::KP_ADD        :
		case bbe::Key::KP_ENTER      :
		case bbe::Key::KP_EQUAL      :
		case bbe::Key::LEFT_SHIFT    :
		case bbe::Key::LEFT_CONTROL  :
		case bbe::Key::LEFT_ALT      :
		case bbe::Key::LEFT_SUPER    :
		case bbe::Key::RIGHT_SHIFT   :
		case bbe::Key::RIGHT_CONTROL :
		case bbe::Key::RIGHT_ALT     :
		case bbe::Key::RIGHT_SUPER   :
		case bbe::Key::MENU          :
			return true;
	}
	return false;
}
