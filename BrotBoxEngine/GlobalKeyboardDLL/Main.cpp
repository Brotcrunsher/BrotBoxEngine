#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

extern "C"
{
#pragma data_seg("shared")
    struct KeyboardEvent
    {
        int    code = 0;
        WPARAM wParam = 0;
        LPARAM lParam = 0;
        DWORD  pid = 0;
    };
#define EVENTS_BUFFER_SIZE 1024
    KeyboardEvent eventsBuffer[EVENTS_BUFFER_SIZE];
    size_t writeHead = 0;
#pragma data_seg()
#pragma comment(linker, "/section:shared,RWS")
    HANDLE getMutex()
    {
        HANDLE retVal = CreateMutexA(nullptr, false, "Global\\BrotBoxEngine\\GlobalKeyboard\\Mutex");
        WaitForSingleObject(retVal, INFINITE);
        return retVal;
    }

    void freeMutex(HANDLE handle)
    {
        ReleaseMutex(handle);
        CloseHandle(handle);
    }

    __declspec(dllexport) LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
    {
        auto mutex = getMutex();
        eventsBuffer[writeHead].code = code;
        eventsBuffer[writeHead].wParam = wParam;
        eventsBuffer[writeHead].lParam = lParam;
        eventsBuffer[writeHead].pid = GetCurrentProcessId();
        writeHead++;
        if (writeHead >= EVENTS_BUFFER_SIZE) writeHead = 0;
        freeMutex(mutex);
        return CallNextHookEx(0, code, wParam, lParam);
    }

    /*Not shared*/ size_t readHead = 0;

    __declspec(dllexport) BOOL WINAPI GetNextEvent(int* code, WPARAM *wParam, LPARAM *lParam, DWORD *pid)
    {
        auto mutex = getMutex();
        if (readHead == writeHead)
        {
            freeMutex(mutex);
            return FALSE;
        }

        *code   = eventsBuffer[readHead].code;
        *wParam = eventsBuffer[readHead].wParam;
        *lParam = eventsBuffer[readHead].lParam;
        *pid    = eventsBuffer[readHead].pid;
        readHead++;
        if (readHead >= EVENTS_BUFFER_SIZE) readHead = 0;
        freeMutex(mutex);
        return TRUE;
    }

    __declspec(dllexport) BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,
        DWORD fdwReason,
        LPVOID lpvReserved)
    {
        if (fdwReason == DLL_PROCESS_ATTACH)
        {
            auto mutex = getMutex();
            readHead = writeHead;
            freeMutex(mutex);
        }

        return TRUE;
    }
}
