#include "EMProcess.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <AtlBase.h>
#include <UIAutomation.h>

void Process::serialDescription(bbe::SerializedDescription& desc) const
{
	desc.describe(title);
	desc.describe(type);
}

void SubsystemProcess::update()
{
	EVERY_SECONDS(10)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);
		BOOL hasEntry = Process32First(snapshot, &entry);
		m_isGameOn = false;
		while (hasEntry)
		{
			bool found = false;
			for (size_t i = 0; i < processes.getLength(); i++)
			{
				if (processes[i].title == entry.szExeFile)
				{
					if (processes[i].type == Process::TYPE_GAME) m_isGameOn = true;
					found = true;
					break;
				}
			}
			if (!found)
			{
				Process newProcess;
				newProcess.title = entry.szExeFile;
				processes.add(newProcess);
			}
			hasEntry = Process32Next(snapshot, &entry);
		}
		CloseHandle(snapshot);
	}
}

bool SubsystemProcess::isGameOn() const
{
	return m_isGameOn;
}

void SubsystemProcess::drawGui()
{
	static bool showSystem = false;
	ImGui::Checkbox("Show System", &showSystem);
	static bool showOther = false;
	ImGui::SameLine();
	ImGui::Checkbox("Show Other", &showOther);
	static bool showGames = false;
	ImGui::SameLine();
	ImGui::Checkbox("Show Games", &showGames);
	if (ImGui::BeginTable("tableProcesses", 2, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 600);
		ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 250);
		bool processChanged = false;
		for (size_t i = 0; i < processes.getLength(); i++)
		{
			Process& p = processes[i];
			if (p.type == Process::TYPE_SYSTEM && !showSystem) continue;
			if (p.type == Process::TYPE_OTHER && !showOther) continue;
			if (p.type == Process::TYPE_GAME && !showGames) continue;
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(p.title);
	
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("S"))
			{
				processChanged = true;
				p.type = Process::TYPE_SYSTEM;
			}
			ImGui::bbe::tooltip("System");
			ImGui::SameLine();
			if (ImGui::Button("O"))
			{
				processChanged = true;
				p.type = Process::TYPE_OTHER;
			}
			ImGui::bbe::tooltip("Other");
			ImGui::SameLine();
			if (ImGui::Button("G"))
			{
				processChanged = true;
				p.type = Process::TYPE_GAME;
			}
			ImGui::bbe::tooltip("Game");
			ImGui::PopID();
		}
		if (processChanged)
		{
			processes.writeToFile();
		}
		ImGui::EndTable();
	}
}

