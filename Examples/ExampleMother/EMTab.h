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
	const Tab* tab = nullptr;
};

DrawTabResult drawTabs(const bbe::List<Tab>& mainTabs,
					   const bbe::List<Tab>& adaptiveTabs,
					   const bbe::List<Tab>& superAdaptiveTabs,
					   bool showAdaptiveTabsInMainWindow,
					   bool showSuperAdaptiveTabsInMainWindow,
					   size_t* previousShownTab,
					   bool& switchLeft,
					   bool& switchRight);

DrawTabResult drawTabs(const bbe::List<Tab>& tabs,
					   size_t* previousShownTab,
					   bool& switchLeft,
					   bool& switchRight);
