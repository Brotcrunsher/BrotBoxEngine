#ifdef WIN32
#include "../BBE/Monitor.h"
#include "../BBE/Logging.h"
#include "../BBE/Math.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <atomic>
#include "BBE/SimpleThread.h"

#pragma comment(lib, "Dxva2.lib")

static void internalSetBrightness(bbe::List<float> &brightnessPercentag)
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
    if (!EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData) -> BOOL {
        MONITORINFOEX monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        if (!GetMonitorInfo(hMonitor, &monitorInfo))
        {
            return true;
        }

        MonitorInfos& mi = *((MonitorInfos*)dwData);
        std::lock_guard _(mi.m);
        mi.monitorInfos.add(monitorInfo);

        return true;
    }, (LPARAM)&monitorInfos))
    {
        BBELOGLN("Failed to enumerate display monitors for MonitorInfos");
    }
    monitorInfos.monitorInfos.sort([](const MONITORINFOEX& a, const MONITORINFOEX& b)
        {
            if (a.rcMonitor.right < b.rcMonitor.right) return true;
            if (a.rcMonitor.right > b.rcMonitor.right) return false;
            if (a.rcMonitor.top < b.rcMonitor.top) return true;
            if (a.rcMonitor.top > b.rcMonitor.top) return false;
            if (a.rcMonitor.bottom < b.rcMonitor.bottom) return true;
            if (a.rcMonitor.bottom > b.rcMonitor.bottom) return false;
            if (a.rcMonitor.right < b.rcMonitor.right) return true;
            if (a.rcMonitor.right > b.rcMonitor.right) return false;

            return false;
        });

    for (size_t i = 0; i < brightnessPercentag.getLength(); i++)
    {
        if (brightnessPercentag[i] > 1.0f) brightnessPercentag[i] = 1.0f;
        if (brightnessPercentag[i] < 0.0f) brightnessPercentag[i] = 0.0f;
        monitorInfos.brightness.add(brightnessPercentag[i] * 10000);
    }

    if (!EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData) -> BOOL {
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

        return true;
        }, (LPARAM)&monitorInfos))
    {
        BBELOGLN("Failed to enumerate display monitors for brightness");
    }
}

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
        internalSetBrightness(targetBrightnessCopy);
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

void bbe::Monitor::setBrightness(const bbe::List<float>& brightnessPercentag)
{
    for (size_t i = 0; i < brightnessPercentag.getLength(); i++)
    {
        if (bbe::Math::isNaN(brightnessPercentag[i])) bbe::Crash(bbe::Error::IllegalArgument, "NaN in brightnessPercentage1");
    }

    std::unique_lock lk(mutex);
    if (targetBrightness != brightnessPercentag)
    {
        targetBrightness = brightnessPercentag;
        lk.unlock();
        gate.notify_one();
    }
}

#endif
