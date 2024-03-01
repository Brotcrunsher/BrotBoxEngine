#pragma once

#include "EMTab.h"

bbe::Vector2 drawTabs(const bbe::List<Tab>& tabs, size_t* previousShownTab, bool& switchLeft, bool& switchRight)
{
	bbe::Vector2 sizeMult(1.0f, 1.0f);
	if (ImGui::BeginTabBar("MainWindowTabs")) {
		size_t desiredShownTab = 0;
		bool programaticTabSwitch = false;
		if (previousShownTab)
		{
			desiredShownTab = *previousShownTab;
			programaticTabSwitch = switchLeft || switchRight;
			if (programaticTabSwitch)
			{
				if (switchLeft) desiredShownTab--;
				if (desiredShownTab == (size_t)-1) desiredShownTab = tabs.getLength() - 1;
				if (switchRight) desiredShownTab++;
				if (desiredShownTab == tabs.getLength()) desiredShownTab = 0;
			}
			switchLeft = false;
			switchRight = false;
		}
		for (size_t i = 0; i < tabs.getLength(); i++)
		{
			const Tab& t = tabs[i];
			if (ImGui::BeginTabItem(t.title, nullptr, (programaticTabSwitch && i == desiredShownTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
			{
				if (previousShownTab) *previousShownTab = i;
				sizeMult = t.run();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	return sizeMult;
}
