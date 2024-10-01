#ifdef WIN32
#include "../BBE/Monitor.h"
#include "../BBE/Logging.h"
#include "../BBE/Math.h"

#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <atomic>

#pragma comment(lib, "Dxva2.lib")

static void internalSetBrightness(bbe::List<float> &brightnessPercentag)
{
    struct Data
    {
        std::atomic<size_t> counter = 0;
        bbe::List<DWORD> brightness;
    };
    Data data;
    for (size_t i = 0; i < brightnessPercentag.getLength(); i++)
    {
        if (brightnessPercentag[i] > 1.0f) brightnessPercentag[i] = 1.0f;
        if (brightnessPercentag[i] < 0.0f) brightnessPercentag[i] = 0.0f;
        data.brightness.add(brightnessPercentag[i] * 10000);
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

        Data& data = *((Data*)dwData);
        for (DWORD i = 0; i < numPhysicalMonitors; i++)
        {
            DWORD min, cur, max;
            if (GetMonitorBrightness(physicalMonitors[i].hPhysicalMonitor, &min, &cur, &max))
            {
                size_t dataAccess = data.counter.fetch_add(1);
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
        }, (LPARAM)&data))
    {
        BBELOGLN("Failed to enumerate display monitors");
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
        // TODO: We should not have to call this multiple times, but for some reason, during tests, only some monitors update. Calling it 2 times
        //       was sufficient, the third time is just for extra paranoia sake.
        for (int i = 0; i < 3; i++) internalSetBrightness(targetBrightnessCopy);
    }
}

bbe::Monitor::Monitor()
{
    setBrightness({ 1.0f });
    thread = std::thread(&bbe::Monitor::threadMain, this);
}

bbe::Monitor::~Monitor()
{
    {
        std::lock_guard _(mutex);
        stopRequested = true;
    }
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
