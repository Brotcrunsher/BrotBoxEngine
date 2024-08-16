#pragma once

#include "EMTab.h"

DrawTabResult drawTabs(const bbe::List<Tab>& tabs, size_t* previousShownTab, bool& switchLeft, bool& switchRight)
{
	bbe::Vector2 sizeMult(1.0f, 1.0f);
	Tab tabby;
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
			const bool tabSelected = ImGui::BeginTabItem(t.title, nullptr, (programaticTabSwitch && i == desiredShownTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None);
			if (t.tooltip[0])
			{
				ImGui::bbe::tooltip(t.tooltip);
			}
			
			if (tabSelected)
			{
				if (previousShownTab) *previousShownTab = i;
				sizeMult = t.run();
				tabby = t;
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	return { sizeMult, tabby };
}
