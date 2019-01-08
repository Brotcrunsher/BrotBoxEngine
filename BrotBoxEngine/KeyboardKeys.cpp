#include "stdafx.h"
#include "BBE/KeyboardKeys.h"
#include "BBE/Exceptions.h"

bbe::String bbe::keyCodeToString(bbe::Key key)
{
	switch (key)
	{
		case bbe::Key::SPACE         :
			return bbe::String("SPACE");
		case bbe::Key::APOSTROPHE    :
			return bbe::String("APOSTROPHE");
		case bbe::Key::COMMA         :
			return bbe::String("COMMA");
		case bbe::Key::MINUS         :
			return bbe::String("MINUS");
		case bbe::Key::PERIOD        :
			return bbe::String("PERIOD");
		case bbe::Key::SLASH         :
			return bbe::String("SLASH");
		case bbe::Key::_0            :
			return bbe::String("0");
		case bbe::Key::_1            :
			return bbe::String("1");
		case bbe::Key::_2            :
			return bbe::String("2");
		case bbe::Key::_3            :
			return bbe::String("3");
		case bbe::Key::_4            :
			return bbe::String("4");
		case bbe::Key::_5            :
			return bbe::String("5");
		case bbe::Key::_6            :
			return bbe::String("6");
		case bbe::Key::_7            :
			return bbe::String("7");
		case bbe::Key::_8            :
			return bbe::String("8");
		case bbe::Key::_9            :
			return bbe::String("9");
		case bbe::Key::SEMICOLON     :
			return bbe::String("SEMICOLON");
		case bbe::Key::EQUAL         :
			return bbe::String("EQUAL");
		case bbe::Key::A             :
			return bbe::String("A");
		case bbe::Key::B             :
			return bbe::String("B");
		case bbe::Key::C             :
			return bbe::String("C");
		case bbe::Key::D             :
			return bbe::String("D");
		case bbe::Key::E             :
			return bbe::String("E");
		case bbe::Key::F             :
			return bbe::String("F");
		case bbe::Key::G             :
			return bbe::String("G");
		case bbe::Key::H             :
			return bbe::String("H");
		case bbe::Key::I             :
			return bbe::String("I");
		case bbe::Key::J             :
			return bbe::String("J");
		case bbe::Key::K             :
			return bbe::String("K");
		case bbe::Key::L             :
			return bbe::String("L");
		case bbe::Key::M             :
			return bbe::String("M");
		case bbe::Key::N             :
			return bbe::String("N");
		case bbe::Key::O             :
			return bbe::String("O");
		case bbe::Key::P             :
			return bbe::String("P");
		case bbe::Key::Q             :
			return bbe::String("Q");
		case bbe::Key::R             :
			return bbe::String("R");
		case bbe::Key::S             :
			return bbe::String("S");
		case bbe::Key::T             :
			return bbe::String("T");
		case bbe::Key::U             :
			return bbe::String("U");
		case bbe::Key::V             :
			return bbe::String("V");
		case bbe::Key::W             :
			return bbe::String("W");
		case bbe::Key::X             :
			return bbe::String("X");
		case bbe::Key::Y             :
			return bbe::String("Y");
		case bbe::Key::Z             :
			return bbe::String("Z");
		case bbe::Key::LEFT_BRACKET  :
			return bbe::String("LEFT_BRACKET");
		case bbe::Key::BACKSLASH     :
			return bbe::String("BACKSLASH");
		case bbe::Key::RIGHT_BRACKET :
			return bbe::String("RIGHT_BRACKET");
		case bbe::Key::GRAVE_ACCENT  :
			return bbe::String("GRAVE_ACCENT");
		case bbe::Key::WORLD_1       :
			return bbe::String("WORLD_1");
		case bbe::Key::WORLD_2       :
			return bbe::String("WORLD_2");
		case bbe::Key::ESCAPE        :
			return bbe::String("ESCAPE");
		case bbe::Key::ENTER         :
			return bbe::String("ENTER");
		case bbe::Key::TAB           :
			return bbe::String("TAB");
		case bbe::Key::BACKSPACE     :
			return bbe::String("BACKSPACE");
		case bbe::Key::INSERT        :
			return bbe::String("INSERT");
		case bbe::Key::DELETE        :
			return bbe::String("DELETE");
		case bbe::Key::RIGHT         :
			return bbe::String("RIGHT");
		case bbe::Key::LEFT          :
			return bbe::String("LEFT");
		case bbe::Key::DOWN          :
			return bbe::String("DOWN");
		case bbe::Key::UP            :
			return bbe::String("UP");
		case bbe::Key::PAGE_UP       :
			return bbe::String("PAGE_UP");
		case bbe::Key::PAGE_DOWN     :
			return bbe::String("PAGE_DOWN");
		case bbe::Key::HOME          :
			return bbe::String("HOME");
		case bbe::Key::END           :
			return bbe::String("END");
		case bbe::Key::CAPS_LOCK     :
			return bbe::String("CAPS_LOCK");
		case bbe::Key::SCROLL_LOCK   :
			return bbe::String("SCROLL_LOCK");
		case bbe::Key::NUM_LOCK      :
			return bbe::String("NUM_LOCK");
		case bbe::Key::PRINT_SCREEN  :
			return bbe::String("PRINT_SCREEN");
		case bbe::Key::PAUSE         :
			return bbe::String("PAUSE");
		case bbe::Key::F1            :
			return bbe::String("F1");
		case bbe::Key::F2            :
			return bbe::String("F2");
		case bbe::Key::F3            :
			return bbe::String("F3");
		case bbe::Key::F4            :
			return bbe::String("F4");
		case bbe::Key::F5            :
			return bbe::String("F5");
		case bbe::Key::F6            :
			return bbe::String("F6");
		case bbe::Key::F7            :
			return bbe::String("F7");
		case bbe::Key::F8            :
			return bbe::String("F8");
		case bbe::Key::F9            :
			return bbe::String("F9");
		case bbe::Key::F10           :
			return bbe::String("F10");
		case bbe::Key::F11           :
			return bbe::String("F11");
		case bbe::Key::F12           :
			return bbe::String("F12");
		case bbe::Key::F13           :
			return bbe::String("F13");
		case bbe::Key::F14           :
			return bbe::String("F14");
		case bbe::Key::F15           :
			return bbe::String("F15");
		case bbe::Key::F16           :
			return bbe::String("F16");
		case bbe::Key::F17           :
			return bbe::String("F17");
		case bbe::Key::F18           :
			return bbe::String("F18");
		case bbe::Key::F19           :
			return bbe::String("F19");
		case bbe::Key::F20           :
			return bbe::String("F20");
		case bbe::Key::F21           :
			return bbe::String("F21");
		case bbe::Key::F22           :
			return bbe::String("F22");
		case bbe::Key::F23           :
			return bbe::String("F23");
		case bbe::Key::F24           :
			return bbe::String("F24");
		case bbe::Key::F25           :
			return bbe::String("F25");
		case bbe::Key::KP_0          :
			return bbe::String("KP_0");
		case bbe::Key::KP_1          :
			return bbe::String("KP_1");
		case bbe::Key::KP_2          :
			return bbe::String("KP_2");
		case bbe::Key::KP_3          :
			return bbe::String("KP_3");
		case bbe::Key::KP_4          :
			return bbe::String("KP_4");
		case bbe::Key::KP_5          :
			return bbe::String("KP_5");
		case bbe::Key::KP_6          :
			return bbe::String("KP_6");
		case bbe::Key::KP_7          :
			return bbe::String("KP_7");
		case bbe::Key::KP_8          :
			return bbe::String("KP_8");
		case bbe::Key::KP_9          :
			return bbe::String("KP_9");
		case bbe::Key::KP_DECIMAL    :
			return bbe::String("KP_DECIMAL");
		case bbe::Key::KP_DIVIDE     :
			return bbe::String("KP_DIVIDE");
		case bbe::Key::KP_MULTIPLY   :
			return bbe::String("KP_MULTIPLY");
		case bbe::Key::KP_SUBTRACT   :
			return bbe::String("KP_SUBTRACT");
		case bbe::Key::KP_ADD        :
			return bbe::String("KP_ADD");
		case bbe::Key::KP_ENTER      :
			return bbe::String("KP_ENTER");
		case bbe::Key::KP_EQUAL      :
			return bbe::String("KP_EQUAL");
		case bbe::Key::LEFT_SHIFT    :
			return bbe::String("LEFT_SHIFT");
		case bbe::Key::LEFT_CONTROL  :
			return bbe::String("LEFT_CONTROL");
		case bbe::Key::LEFT_ALT      :
			return bbe::String("LEFT_ALT");
		case bbe::Key::LEFT_SUPER    :
			return bbe::String("LEFT_SUPER");
		case bbe::Key::RIGHT_SHIFT   :
			return bbe::String("RIGHT_SHIFT");
		case bbe::Key::RIGHT_CONTROL :
			return bbe::String("RIGHT_CONTROL");
		case bbe::Key::RIGHT_ALT     :
			return bbe::String("RIGHT_ALT");
		case bbe::Key::RIGHT_SUPER   :
			return bbe::String("RIGHT_SUPER");
		case bbe::Key::MENU          :
			return bbe::String("MENU");
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
