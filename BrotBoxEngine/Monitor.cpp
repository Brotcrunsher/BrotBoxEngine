#include "../BBE/Monitor.h"
#include "../BBE/Logging.h"
#include "../BBE/Math.h"
#include "BBE/SimpleThread.h"

#include <cmath>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>

#pragma comment(lib, "Dxva2.lib")

namespace
{
	void internalSetBrightnessWindows(bbe::List<float> &brightnessPercentag)
	{
		// First we have to get all the monitor infos and then sort them so that we later know which brightnessPercentage is for which monitor.
		// If we don't do this, then the order is rather random, as the order in which EnumDisplayMonitors iterates is not specified and can change
		// between calls.
		struct MonitorInfos
		{
			std::mutex m;
			bbe::List<MONITORINFOEX> monitorInfos;
			bbe::List<DWORD> brightness;
		};
		MonitorInfos monitorInfos;
		if (!EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData) -> BOOL
								 {
			MONITORINFOEX monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			if (!GetMonitorInfo(hMonitor, &monitorInfo))
			{
				return true;
			}

			MonitorInfos& mi = *((MonitorInfos*)dwData);
			std::lock_guard _(mi.m);
			mi.monitorInfos.add(monitorInfo);

			return true; }, (LPARAM)&monitorInfos))
		{
			BBELOGLN("Failed to enumerate display monitors for MonitorInfos");
		}
		monitorInfos.monitorInfos.sort([](const MONITORINFOEX &a, const MONITORINFOEX &b)
									   {
			if (a.rcMonitor.right < b.rcMonitor.right) return true;
			if (a.rcMonitor.right > b.rcMonitor.right) return false;
			if (a.rcMonitor.top < b.rcMonitor.top) return true;
			if (a.rcMonitor.top > b.rcMonitor.top) return false;
			if (a.rcMonitor.bottom < b.rcMonitor.bottom) return true;
			if (a.rcMonitor.bottom > b.rcMonitor.bottom) return false;
			if (a.rcMonitor.right < b.rcMonitor.right) return true;
			if (a.rcMonitor.right > b.rcMonitor.right) return false;

			return false; });

		for (size_t i = 0; i < brightnessPercentag.getLength(); i++)
		{
			if (brightnessPercentag[i] > 1.0f) brightnessPercentag[i] = 1.0f;
			if (brightnessPercentag[i] < 0.0f) brightnessPercentag[i] = 0.0f;
			monitorInfos.brightness.add(brightnessPercentag[i] * 10000);
		}

		if (!EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData) -> BOOL
								 {
			DWORD numPhysicalMonitors = 0;

			if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numPhysicalMonitors))
			{
				BBELOGLN("Failed to get number of physical monitors");
				return true;
			}

			bbe::List<PHYSICAL_MONITOR> physicalMonitors;
			physicalMonitors.resizeCapacityAndLength(numPhysicalMonitors);

			if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, numPhysicalMonitors, physicalMonitors.getRaw()))
			{
				BBELOGLN("Failed to get physical monitors");
				return true;
			}

			MonitorInfos& data = *((MonitorInfos*)dwData);
			for (DWORD i = 0; i < numPhysicalMonitors; i++)
			{
				DWORD min, cur, max;
				if (GetMonitorBrightness(physicalMonitors[i].hPhysicalMonitor, &min, &cur, &max))
				{
					size_t dataAccess = 0;
					MONITORINFOEX monitorInfo;
					monitorInfo.cbSize = sizeof(MONITORINFOEX);
					if (GetMonitorInfo(hMonitor, &monitorInfo))
					{
						for (size_t k = 0; k < data.monitorInfos.getLength(); k++)
						{
							if (data.monitorInfos[k].rcMonitor.right     == monitorInfo.rcMonitor.right
								&& data.monitorInfos[k].rcMonitor.left   == monitorInfo.rcMonitor.left
								&& data.monitorInfos[k].rcMonitor.top    == monitorInfo.rcMonitor.top
								&& data.monitorInfos[k].rcMonitor.bottom == monitorInfo.rcMonitor.bottom)
							{
								dataAccess = k;
								break;
							}
						}
					}
					if (dataAccess >= data.brightness.getLength()) dataAccess = 0;

					float percentage = data.brightness[dataAccess] / 10000.f;

					DWORD val = (DWORD)((max - min) * percentage + min);
					// Might not be necessary, but I want to make 100% sure no floating point shenanigans happen
					if (data.brightness[dataAccess] == 10000) val = max;
					if (data.brightness[dataAccess] == 0) val = min;

					if (!SetMonitorBrightness(physicalMonitors[i].hPhysicalMonitor, val))
					{
						BBELOGLN("Failed to set brightness for monitor " << i);
					}
				}
			}
			DestroyPhysicalMonitors(numPhysicalMonitors, physicalMonitors.getRaw());

			return true; }, (LPARAM)&monitorInfos))
		{
			BBELOGLN("Failed to enumerate display monitors for brightness");
		}
	}
}
#elif defined(__linux__)
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace
{
	struct ProcessResult
	{
		bool launched = false;
		int exitCode = -1;
		std::string output;
	};

	ProcessResult runCommand(const std::vector<std::string> &args)
	{
		ProcessResult result;
		if (args.empty())
		{
			return result;
		}

		int pipeFds[2] = { -1, -1 };
		if (::pipe(pipeFds) != 0)
		{
			return result;
		}

		const pid_t child = fork();
		if (child == 0)
		{
			dup2(pipeFds[1], STDOUT_FILENO);
			dup2(pipeFds[1], STDERR_FILENO);
			close(pipeFds[0]);
			close(pipeFds[1]);

			std::vector<char *> argv;
			argv.reserve(args.size() + 1);
			for (const auto &arg : args)
			{
				argv.push_back(const_cast<char *>(arg.c_str()));
			}
			argv.push_back(nullptr);

			execvp(argv[0], argv.data());
			_exit(errno == ENOENT ? 127 : 126);
		}

		close(pipeFds[1]);

		if (child < 0)
		{
			close(pipeFds[0]);
			return result;
		}

		result.launched = true;
		char buffer[512];
		ssize_t bytesRead = 0;
		while ((bytesRead = read(pipeFds[0], buffer, sizeof(buffer))) > 0)
		{
			result.output.append(buffer, buffer + bytesRead);
		}
		close(pipeFds[0]);

		int status = 0;
		if (waitpid(child, &status, 0) >= 0 && WIFEXITED(status))
		{
			result.exitCode = WEXITSTATUS(status);
		}

		return result;
	}

	void reportLinuxMonitorFailureOnce(bool &alreadyReported, const std::string &message, const std::string &details = {})
	{
		if (alreadyReported)
		{
			return;
		}

		alreadyReported = true;
		if (details.empty())
		{
			BBELOGLN(message);
		}
		else
		{
			BBELOGLN(message << ": " << details);
		}
	}

	bbe::List<int> detectLinuxDisplays(bool &backendReportedUnavailable)
	{
		bbe::List<int> displays;
		const auto result = runCommand({ "ddcutil", "detect", "--brief" });
		if (!result.launched)
		{
			reportLinuxMonitorFailureOnce(backendReportedUnavailable, "Linux monitor dimming unavailable, failed to launch ddcutil");
			return displays;
		}

		if (result.exitCode != 0)
		{
			reportLinuxMonitorFailureOnce(backendReportedUnavailable, "Linux monitor dimming unavailable, ddcutil detect --brief failed", result.output);
			return displays;
		}

		std::istringstream stream(result.output);
		std::string line;
		while (std::getline(stream, line))
		{
			int displayNumber = 0;
			if (std::sscanf(line.c_str(), "Display %d", &displayNumber) == 1)
			{
				displays.add(displayNumber);
			}
		}

		if (displays.isEmpty())
		{
			reportLinuxMonitorFailureOnce(backendReportedUnavailable, "Linux monitor dimming unavailable, ddcutil did not report any controllable displays");
		}

		return displays;
	}

	void internalSetBrightnessLinux(const bbe::List<float> &brightnessPercentag, const bbe::List<int> &displays, bool &backendReportedUnavailable)
	{
		for (size_t i = 0; i < brightnessPercentag.getLength() && i < displays.getLength(); i++)
		{
			const int brightness = (int)std::lround(brightnessPercentag[i] * 100.0f);
			const auto result = runCommand({
				"ddcutil",
				"setvcp",
				"--display",
				std::to_string(displays[i]),
				"10",
				std::to_string(brightness),
				"--noverify",
			});

			if (!result.launched)
			{
				reportLinuxMonitorFailureOnce(backendReportedUnavailable, "Linux monitor dimming unavailable, failed to launch ddcutil");
				return;
			}

			if (result.exitCode != 0)
			{
				reportLinuxMonitorFailureOnce(backendReportedUnavailable, "Linux monitor dimming failed while setting brightness via ddcutil", result.output);
				return;
			}
		}
	}
}
#endif

void bbe::Monitor::threadMain()
{
	while (true)
	{
		std::unique_lock lk(mutex);
		while (currentBrightness == targetBrightness && !stopRequested)
		{
			gate.wait(lk);
		}
		if (stopRequested) return;
		currentBrightness = targetBrightness;
		bbe::List<float> targetBrightnessCopy = targetBrightness; // So that we can change the brightness without being under the lock
		lk.unlock();

#ifdef WIN32
		internalSetBrightnessWindows(targetBrightnessCopy);
#elif defined(__linux__)
		if (!linuxDisplaysInitialized)
		{
			linuxDisplays = detectLinuxDisplays(linuxBackendReportedUnavailable);
			linuxDisplaysInitialized = true;
		}
		if (!linuxDisplays.isEmpty())
		{
			internalSetBrightnessLinux(targetBrightnessCopy, linuxDisplays, linuxBackendReportedUnavailable);
		}
#else
		(void)targetBrightnessCopy;
#endif
	}
}

bbe::Monitor::Monitor()
{
	thread = std::thread(&bbe::Monitor::threadMain, this);
	bbe::simpleThread::setName(thread, "BBE Monitor");
}

bbe::Monitor::~Monitor()
{
	stopRequested = true;
	gate.notify_one();
	thread.join();
}

void bbe::Monitor::setBrightness(const bbe::List<float> &brightnessPercentag)
{
	bbe::List<float> normalizedBrightness = brightnessPercentag;
	for (size_t i = 0; i < normalizedBrightness.getLength(); i++)
	{
		if (bbe::Math::isNaN(normalizedBrightness[i])) bbe::Crash(bbe::Error::IllegalArgument, "NaN in brightnessPercentage1");

		if (normalizedBrightness[i] > 1.0f) normalizedBrightness[i] = 1.0f;
		if (normalizedBrightness[i] < 0.0f) normalizedBrightness[i] = 0.0f;
#ifdef __linux__
		normalizedBrightness[i] = std::round(normalizedBrightness[i] * 100.0f) / 100.0f;
#endif
	}

	std::unique_lock lk(mutex);
	if (targetBrightness != normalizedBrightness)
	{
		targetBrightness = normalizedBrightness;
		lk.unlock();
		gate.notify_one();
	}
}
