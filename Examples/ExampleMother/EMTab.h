#pragma once

#include "BBE/BrotBoxEngine.h"

struct Tab
{
	const char* title = "";
	const char* tooltip = "";
	std::function<bbe::Vector2()> run;
};

struct DrawTabResult
{
	bbe::Vector2 sizeMult;
	Tab tab;
};

DrawTabResult drawTabs(const bbe::List<Tab>& tabs, size_t* previousShownTab, bool& switchLeft, bool& switchRight);
