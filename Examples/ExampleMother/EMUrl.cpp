#include "EMUrl.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "AssetStore.h"
#define NOMINMAX
#include <Windows.h>
#include <tlhelp32.h>
#include <AtlBase.h>
#include <UIAutomation.h>

void Url::serialize(bbe::ByteBuffer& buffer) const
{
	buffer.write(url);
	buffer.write(type);
}

Url Url::deserialize(bbe::ByteBufferSpan& buffer)
{
	Url retVal;

	buffer.read(retVal.url);
	buffer.read(retVal.type);

	return retVal;
}

void SubsystemUrl::update()
{
	m_timeWasterUrlFound = false;
	EVERY_SECONDS(1)
	{
		auto tabNames = getDomains();
		for (size_t i = 0; i < tabNames.getLength(); i++)
		{
			bool found = false;
			for (size_t k = 0; k < urls.getLength(); k++)
			{
				if (tabNames[i] == urls[k].url)
				{
					found = true;
					if (urls[k].type == Url::TYPE_TIME_WASTER)
					{
						m_timeWasterUrlFound = true;
					}
					break;
				}
			}
			if (!found)
			{
				Url newUrl;
				newUrl.url = tabNames[i].getRaw();
				urls.add(newUrl);
			}
		}
	}
}

void SubsystemUrl::drawGui()
{
	static bool showTimeWasters = false;
	ImGui::Checkbox("Show Time Wasters", &showTimeWasters);
	static bool showWork = false;
	ImGui::SameLine();
	ImGui::Checkbox("Show Work", &showWork);
	if (ImGui::BeginTable("tableUrls", 2, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 600);
		ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
		bool urlChanged = false;
		size_t deletionIndex = (size_t)-1;
		for (size_t i = 0; i < urls.getLength(); i++)
		{
			Url& url = urls[i];
			if (url.type == Url::TYPE_TIME_WASTER && !showTimeWasters) continue;
			if (url.type == Url::TYPE_WORK        && !showWork) continue;
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(url.url);
	
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("Delete"))
			{
				deletionIndex = i;
			}
			ImGui::SameLine();
			if (ImGui::Button("T"))
			{
				urlChanged = true;
				url.type = Url::TYPE_TIME_WASTER;
			}
			ImGui::bbe::tooltip("Time Waster");
			ImGui::SameLine();
			if (ImGui::Button("W"))
			{
				urlChanged = true;
				url.type = Url::TYPE_WORK;
			}
			ImGui::bbe::tooltip("Work");
			ImGui::PopID();
		}
		if (urlChanged)
		{
			urls.writeToFile();
		}
		urls.removeIndex(deletionIndex);
		ImGui::EndTable();
	}
}

bool SubsystemUrl::timeWasterFound() const
{
	return m_timeWasterUrlFound;
}

bbe::List<bbe::String> SubsystemUrl::getDomains()
{
	bbe::List<bbe::String> retVal;
	//Inspired By: Barmak Shemirani see https://stackoverflow.com/a/48507146/7130273
	//Modified to return a bbe::List<bbe::String>
	//         only the domain part
	//         performance optimized (caching etc.)

	static bool iniDone = false;
	static CComQIPtr<IUIAutomation> uia;
	static CComPtr<IUIAutomationCondition> condition;
	if (!iniDone)
	{
		iniDone = true;
		if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia)
			return retVal;

		//uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
		//	CComVariant(0xC354), &condition);
		// TODO: Localization. We should have something like the above 2 lines, but unfortunately
		//       the layout of chrome seems to have changed quite a bit and some random UI Elements
		//       sometimes fulfill the condition.
		uia->CreatePropertyCondition(UIA_NamePropertyId,
			CComVariant(L"Adress- und Suchleiste"), &condition);
	}

	static bool redGreen = true;
	redGreen = !redGreen;
	struct Edit
	{
		CComPtr<IUIAutomationElement> edit;
		bool currentRedGreen = redGreen;
	};
	static std::map<HWND, Edit> editsCache;
	{
		HWND hwnd = NULL;
		while (true)
		{
			hwnd = FindWindowEx(NULL, hwnd, "Chrome_WidgetWin_1", NULL);
			if (!hwnd)
				break;
			if (!IsWindowVisible(hwnd))
				continue;
			if (GetWindowTextLength(hwnd) == 0)
				continue;

			if (editsCache.count(hwnd) != 0)
			{
				editsCache[hwnd].currentRedGreen = redGreen;
				continue;
			}

			CComPtr<IUIAutomationElement> root;
			if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root)
				continue;

			CComPtr<IUIAutomationElement> edit;
			if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &edit))
				|| !edit)
				continue;
			// ^^^^--- This is the actual reason why we do this cache stuff!
			//         Highly expensive operation to call FindFirst.
			editsCache[hwnd] = { edit, redGreen };
		}
	}

	// Remove cache entries that weren't "touched" in this call.
	for (auto it = editsCache.cbegin(); it != editsCache.cend(); )
	{
		if (it->second.currentRedGreen != redGreen)
		{
			editsCache.erase(it++);
		}
		else
		{
			++it;
		}
	}

	for (auto it = editsCache.begin(); it != editsCache.end(); it++)
	{
		CComPtr<IUIAutomationElement> edit = it->second.edit;

		CComVariant focus;
		edit->GetCurrentPropertyValue(UIA_HasKeyboardFocusPropertyId, &focus);

		if (focus.boolVal)
			continue; // We do not want to get the tab url if it's currently getting modified

		CComVariant url;
		edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
		if (!url.bstrVal) continue;

		bbe::String newElem = bbe::String(url.bstrVal).split("/")[0];
		if (newElem.getLength() > 0
			&& newElem.contains("."))
		{
			retVal.add(newElem);
		}
	}
	return retVal;
}
