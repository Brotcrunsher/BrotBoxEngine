#if defined(_WIN32) || defined(__linux__)
#include "EMProcess.h"
#include "BBE/ImGuiExtensions.h"

#include <filesystem>
#include <unordered_map>

#ifdef _WIN32
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#elif defined(__linux__)
#include <fstream>
#endif

namespace
{
	bool isMotherProcess(const bbe::String &scannedProcessName, const bbe::String &exePath)
	{
		if (scannedProcessName == "ExampleMother.exe" || scannedProcessName == "ExampleMother")
		{
			return true;
		}

		if (!exePath.isEmpty())
		{
			const bbe::String exeName = std::filesystem::path(exePath.getRaw()).filename().string().c_str();
			return exeName == "ExampleMother.exe" || exeName == "ExampleMother";
		}

		return false;
	}

	bool shouldIgnoreProcess(const bbe::String &scannedProcessName)
	{
		return scannedProcessName.startsWith("AM_Delta_Patch_") || scannedProcessName.endsWith("_chrome_updater.exe") || scannedProcessName.startsWith("MicrosoftEdge_") || scannedProcessName.startsWith("kworker/") || scannedProcessName.startsWith("nvidia-drm/timeline-");
	}

	void handleScannedProcess(const bbe::String &scannedProcessName,
							  const bbe::String &exePath,
							  std::unordered_map<std::string, int32_t> &knownProcessTypes,
							  bbe::SerializableList<Process> &processes,
							  bbe::List<bbe::String> &foundGames,
							  int32_t &motherPorcesses)
	{
		if (isMotherProcess(scannedProcessName, exePath))
		{
			motherPorcesses++;
		}

		if (shouldIgnoreProcess(scannedProcessName))
		{
			return;
		}

		auto processIt = knownProcessTypes.find(scannedProcessName.getRaw());
		if (processIt != knownProcessTypes.end())
		{
			if (processIt->second == Process::TYPE_GAME)
			{
				foundGames.add(scannedProcessName);
			}
			return;
		}

		Process newProcess;
		newProcess.title = scannedProcessName;
		newProcess.exePath = exePath;
		processes.add(newProcess);
		knownProcessTypes.emplace(scannedProcessName.getRaw(), newProcess.type);
	}

#ifdef __linux__
	bool isNumericDirectoryName(const std::string &name)
	{
		if (name.empty())
		{
			return false;
		}

		for (char c : name)
		{
			if (c < '0' || c > '9')
			{
				return false;
			}
		}

		return true;
	}

	bbe::String readLinuxProcessName(const std::filesystem::path &procPath)
	{
		std::ifstream file(procPath / "comm");
		if (!file.is_open())
		{
			return "";
		}

		std::string line;
		std::getline(file, line);
		return bbe::String(line.c_str());
	}

	bbe::String readLinuxExePath(const std::filesystem::path &procPath)
	{
		std::error_code ec;
		const auto exePath = std::filesystem::read_symlink(procPath / "exe", ec);
		if (ec)
		{
			return "Failed to read /proc/<pid>/exe";
		}
		return bbe::String(exePath.string().c_str());
	}
#endif
}

void SubsystemProcess::update()
{
	EVERY_SECONDS(10)
	{
		motherPorcesses = 0;
		foundGames.clear();
		std::unordered_map<std::string, int32_t> knownProcessTypes;
		knownProcessTypes.reserve(processes.getLength());
		for (size_t i = 0; i < processes.getLength(); i++)
		{
			knownProcessTypes.emplace(processes[i].title.getRaw(), processes[i].type);
		}
#ifdef _WIN32
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);
		BOOL hasEntry = Process32First(snapshot, &entry);
		while (hasEntry)
		{
			const bbe::String scannedProcessName = entry.szExeFile;
			bbe::String exePath = "Failed to OpenProcess";
			constexpr int32_t MAX_EXE_PATH_LENGTH = 1024;
			char exePathBuffer[MAX_EXE_PATH_LENGTH];
			auto entryHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);
			if (entryHandle)
			{
				if (GetModuleFileNameEx(entryHandle, 0, exePathBuffer, MAX_EXE_PATH_LENGTH))
				{
					exePath = exePathBuffer;
				}
				else
				{
					exePath = "Failed to GetModuleFileNameEx";
				}
				CloseHandle(entryHandle);
			}

			handleScannedProcess(scannedProcessName, exePath, knownProcessTypes, processes, foundGames, motherPorcesses);
			hasEntry = Process32Next(snapshot, &entry);
		}
		CloseHandle(snapshot);
#elif defined(__linux__)
		std::error_code ec;
		for (const auto &entry : std::filesystem::directory_iterator("/proc", ec))
		{
			if (ec)
			{
				break;
			}

			if (!entry.is_directory(ec))
			{
				ec.clear();
				continue;
			}

			const std::string directoryName = entry.path().filename().string();
			if (!isNumericDirectoryName(directoryName))
			{
				continue;
			}

			const bbe::String scannedProcessName = readLinuxProcessName(entry.path());
			if (scannedProcessName.isEmpty())
			{
				continue;
			}

			const bbe::String exePath = readLinuxExePath(entry.path());
			handleScannedProcess(scannedProcessName, exePath, knownProcessTypes, processes, foundGames, motherPorcesses);
		}
#endif
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
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", foundGames[i].getRaw());
	}

	if (ImGui::BeginTable("tableProcesses", 2, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed, 300 * scale);
		ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed, 125 * scale);
		bool processChanged = false;
		for (size_t i = 0; i < processes.getLength(); i++)
		{
			Process &p = processes[i];
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

bbe::List<bbe::String> SubsystemProcess::getWarnings() const
{
	bbe::List<bbe::String> retVal;

	if (motherPorcesses == 0)
	{
		retVal.add("Could not find any mother process.");
	}
	else if (motherPorcesses > 1)
	{
		bbe::String msg = "Found multiple Mother processes: ";
		msg += motherPorcesses;
		retVal.add(msg);
	}

	return retVal;
}
#endif
