#include "EMTab.h"

namespace
{
	size_t countVisibleTabs(const bbe::List<Tab> &tabs)
	{
		return tabs.getLength();
	}
}

DrawTabResult drawTabs(const bbe::List<Tab> &mainTabs,
					   const bbe::List<Tab> &adaptiveTabs,
					   const bbe::List<Tab> &superAdaptiveTabs,
					   bool showAdaptiveTabsInMainWindow,
					   bool showSuperAdaptiveTabsInMainWindow,
					   size_t *previousShownTab,
					   bool &switchLeft,
					   bool &switchRight)
{
	bbe::Vector2 sizeMult(1.0f, 1.0f);
	const Tab *selectedTab = nullptr;
	if (ImGui::BeginTabBar("MainWindowTabs"))
	{
		size_t desiredShownTab = 0;
		bool programaticTabSwitch = false;
		size_t visibleTabCount = countVisibleTabs(mainTabs);
		if (showAdaptiveTabsInMainWindow)
		{
			visibleTabCount += countVisibleTabs(adaptiveTabs);
		}
		if (showSuperAdaptiveTabsInMainWindow)
		{
			visibleTabCount += countVisibleTabs(superAdaptiveTabs);
		}
		if (previousShownTab)
		{
			desiredShownTab = *previousShownTab;
			if (visibleTabCount > 0 && desiredShownTab >= visibleTabCount)
			{
				desiredShownTab = visibleTabCount - 1;
			}
			programaticTabSwitch = switchLeft || switchRight;
			if (programaticTabSwitch && visibleTabCount > 0)
			{
				if (switchLeft) desiredShownTab--;
				if (desiredShownTab == (size_t)-1) desiredShownTab = visibleTabCount - 1;
				if (switchRight) desiredShownTab++;
				if (desiredShownTab == visibleTabCount) desiredShownTab = 0;
			}
			switchLeft = false;
			switchRight = false;
		}

		size_t visibleTabIndex = 0;
		auto drawTabList = [&](const bbe::List<Tab> &tabs)
		{
			for (size_t i = 0; i < tabs.getLength(); i++)
			{
				const Tab &t = tabs[i];
				const bool tabSelected = ImGui::BeginTabItem(t.title, nullptr, (programaticTabSwitch && visibleTabIndex == desiredShownTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None);
				if (t.tooltip[0])
				{
					ImGui::bbe::tooltip(t.tooltip);
				}

				if (tabSelected)
				{
					if (previousShownTab) *previousShownTab = visibleTabIndex;
					sizeMult = t.run();
					selectedTab = &t;
					ImGui::EndTabItem();
				}
				visibleTabIndex++;
			}
		};

		drawTabList(mainTabs);
		if (showAdaptiveTabsInMainWindow)
		{
			drawTabList(adaptiveTabs);
		}
		if (showSuperAdaptiveTabsInMainWindow)
		{
			drawTabList(superAdaptiveTabs);
		}
		ImGui::EndTabBar();
	}
	return { sizeMult, selectedTab };
}

DrawTabResult drawTabs(const bbe::List<Tab> &tabs,
					   size_t *previousShownTab,
					   bool &switchLeft,
					   bool &switchRight)
{
	static const bbe::List<Tab> emptyTabs;
	return drawTabs(tabs, emptyTabs, emptyTabs, false, false, previousShownTab, switchLeft, switchRight);
}
