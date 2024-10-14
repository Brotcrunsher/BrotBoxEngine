#ifdef WIN32
#include "BBE/AdafruitMacroPadRP2040.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>

#include "BBE/String.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

void bbe::AdafruitMacroPadRP2040::threadMain()
{
    while (threadRunning) {
        OVERLAPPED overlapped = { 0 };
        overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!overlapped.hEvent)
        {
            bbe::Crash(bbe::Error::IllegalState, "Failed to create event!");
            return;
        }
        BYTE report[3] = {};
        DWORD bytesRead = 0;
        BOOL result = ReadFile(winHandle, report, sizeof(report), &bytesRead, &overlapped);
        if (!result) {
            if (GetLastError() == ERROR_IO_PENDING) {
                HANDLE events[] = { overlapped.hEvent, stopEvent };
                DWORD waitResult = WaitForMultipleObjects(2, events, false, INFINITE);
                if (waitResult == WAIT_OBJECT_0 + 1)
                {
                    // stopEvent was triggered.
                    CloseHandle(overlapped.hEvent);
                    break;
                }
                GetOverlappedResult(winHandle, &overlapped, &bytesRead, FALSE);
            }
            else {
                CloseHandle(overlapped.hEvent);
                break;
            }
        }

        if (bytesRead >= 3) {
            if (report[0] != 1)
            {
                bbe::String s = "Received strange report ID : ";
                s += report[0];
                bbe::Crash(bbe::Error::IllegalArgument, s.getRaw());
            }
            std::lock_guard _(keyEventsMutex);
            keyEvents.add({ report[1], report[2] });
        }
        CloseHandle(overlapped.hEvent);
    }
    CloseHandle(winHandle);
    threadRunning = false;
}

bbe::AdafruitMacroPadRP2040::AdafruitMacroPadRP2040()
{
    stopEvent = CreateEvent(nullptr, true, false, nullptr);
}

bbe::AdafruitMacroPadRP2040::~AdafruitMacroPadRP2040()
{
    disconnect();
    CloseHandle(stopEvent);
}

bool bbe::AdafruitMacroPadRP2040::connect()
{
	if (isConnected()) return true;

    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return false;
    }

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    DWORD index = 0;
    bool deviceFound = false;

    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, index, &deviceInterfaceData)) {
        index++;
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

        PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
        if (deviceDetail == NULL) {
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
            return false;
        }
        deviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceDetail, requiredSize, NULL, NULL)) {
            HANDLE deviceHandle = CreateFile(deviceDetail->DevicePath, GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

            if (deviceHandle != INVALID_HANDLE_VALUE) {
                HIDD_ATTRIBUTES attributes;
                attributes.Size = sizeof(HIDD_ATTRIBUTES);
                if (HidD_GetAttributes(deviceHandle, &attributes)) {
                    if (attributes.VendorID == 0x239A /* Adafruit VID */ && attributes.ProductID == 0x8108 /* MacroPad PID */) {
                        deviceFound = true;
                        this->winHandle = deviceHandle;
                        threadRunning = true;
                        thread = std::thread(&AdafruitMacroPadRP2040::threadMain, this);
                        break;
                    }
                }
            }
        }
        free(deviceDetail);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return deviceFound;
}

void bbe::AdafruitMacroPadRP2040::disconnect()
{
    if (!isConnected()) return;

    SetEvent(stopEvent);
    thread.join();
}

bool bbe::AdafruitMacroPadRP2040::isConnected() const
{
	return threadRunning;
}

bool bbe::AdafruitMacroPadRP2040::isKeyDown(RP2040Key key) const
{
    return keyDown[(size_t)key];
}

bool bbe::AdafruitMacroPadRP2040::isKeyUp(RP2040Key key) const
{
    return !isKeyDown(key);
}

bool bbe::AdafruitMacroPadRP2040::isKeyPressed(RP2040Key key) const
{
    return keyPressed[(size_t)key];
}

int64_t bbe::AdafruitMacroPadRP2040::getRotationValue() const
{
    return rotationValue;
}

void bbe::AdafruitMacroPadRP2040::update()
{
    bbe::List<KeyEvent> currentEvents;
    {
        std::lock_guard _(keyEventsMutex);
        currentEvents = std::move(keyEvents);
    }

    keyPressed = {};

    for (size_t i = 0; i < currentEvents.getLength(); i++)
    {
        if (currentEvents[i].key == (unsigned char)RP2040Key::BUTTON_ROTATE_AUDIO)
        {
            if (currentEvents[i].pressed == 0)
            {
                rotationValue++;
            }
            else
            {
                rotationValue--;
            }
        }
        else
        {
            if (currentEvents[i].pressed == 0)
            {
                keyDown[currentEvents[i].key] = true;
                keyPressed[currentEvents[i].key] = true;
            }
            else
            {
                keyDown[currentEvents[i].key] = false;
            }
        }
    }
}



#endif
