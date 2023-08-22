#pragma once
#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock.h>
#include <Windows.h>
#include <algorithm> 
#include <Psapi.h>
#include <TlHelp32.h>
#include <vector>

using namespace std;

class WinApiHelper
{
public:
    struct ScreenshotInfo {
        int Height;
        int Width;

        // Contains pixels info in screenshot in following format
        // p1R, p1G, p1B, p2R, p2G, p2B...
        unsigned char* Data;

        ScreenshotInfo(int w, int h, unsigned char* d) {
            Height = h;
            Width = w;
            Data = d;
        }

        ~ScreenshotInfo() {
            delete Data;
        }
    };
    struct WindowInfo {
        HWND Handle;
        string Title;

        WindowInfo(HWND h, string t) {
            Handle = h;
            Title = t;
        }
    };


    static string GetExecutablePath();
    static vector<string> GetIpAddresses();
    static bool AddToStartup(const char* path, const char* startName);
    static bool SetDesktopWallpers(const char* path);
    static string GetDesktopWallpersPath();
    static ScreenshotInfo TakeScreenshot();

    static DWORD GetProcId(HWND windowHandle);
    static DWORD GetProcId(const char* processName);
    static bool KillProcess(DWORD processId, UINT exitCode = 0); 
    static char* GetProcName(DWORD procId);
    static vector<PROCESSENTRY32> GetAllProcesses();
    static vector<WindowInfo> GetAllWindowProcesses();

    static bool MutexExists(const char* name);
    static bool CloseMutex(HANDLE h);
    static HANDLE SetMutex(const char* name);
};
