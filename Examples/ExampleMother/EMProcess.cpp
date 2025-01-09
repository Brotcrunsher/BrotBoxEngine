#include "EMProcess.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include "BBE/ImGuiExtensions.h"

void SubsystemProcess::update()
{
	EVERY_SECONDS(10)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);
		BOOL hasEntry = Process32First(snapshot, &entry);
		foundGames.clear();
		while (hasEntry)
		{
			const bbe::String scannedProcessName = entry.szExeFile;
			if (!scannedProcessName.startsWith("AM_Delta_Patch_") // Microsoft Anti-Malware Signature Delta Update Package
				&& !scannedProcessName.endsWith("_chrome_updater.exe")
				&& !scannedProcessName.startsWith("MicrosoftEdge_")
				) 
			{
				bool found = false;
				for (size_t i = 0; i < processes.getLength(); i++)
				{
					if (processes[i].title == entry.szExeFile)
					{
						if (processes[i].type == Process::TYPE_GAME) foundGames.add(processes[i].title);
						found = true;
					}
				}
				if (!found)
				{
					Process newProcess;
					newProcess.title = entry.szExeFile;
					constexpr int32_t MAX_EXE_PATH_LENGTH = 1024;
					char exePathBuffer[MAX_EXE_PATH_LENGTH];
					auto entryHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);
					if (entryHandle)
					{
						if (GetModuleFileNameEx(entryHandle, 0, exePathBuffer, MAX_EXE_PATH_LENGTH))
						{
							newProcess.exePath = exePathBuffer;
						}
						else
						{
							newProcess.exePath = "Failed to GetModuleFileNameEx";
						}
					}
					else
					{
						newProcess.exePath = "Failed to OpenProcess";
					}
					processes.add(newProcess);
				}
			}
			hasEntry = Process32Next(snapshot, &entry);
		}
		CloseHandle(snapshot);
	}
}

bool SubsystemProcess::isGameOn() const
{
	return !foundGames.isEmpty();
}

void SubsystemProcess::drawGui(float scale)
{
	static bool showSystem = false;
	ImGui::Checkbox("Show System", &showSystem);
	static bool showOther = false;
	ImGui::SameLine();
	ImGui::Checkbox("Show Other", &showOther);
	static bool showGames = false;
	ImGui::SameLine();
	ImGui::Checkbox("Show Games", &showGames);

	for (size_t i = 0; i < foundGames.getLength(); i++)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), foundGames[i].getRaw());
	}

	if (ImGui::BeginTable("tableProcesses", 2, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 300 * scale);
		ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 125 * scale);
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
			ImGui::bbe::tooltip(p.exePath);
	
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

