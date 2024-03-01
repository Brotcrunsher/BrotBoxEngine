#pragma once

#include "BBE/BrotBoxEngine.h"

struct Tab
{
	const char* title = "";
	const char* tooltip = "";
	std::function<bbe::Vector2()> run;
};

bbe::Vector2 drawTabs(const bbe::List<Tab>& tabs, size_t* previousShownTab, bool& switchLeft, bool& switchRight);
