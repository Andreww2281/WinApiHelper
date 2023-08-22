#include "WinApiHelper.h"


bool WinApiHelper::KillProcess(DWORD processId, UINT exitCode)
{
    DWORD DesiredAccess = PROCESS_TERMINATE;
    BOOL  bInheritHandle = FALSE;

    HANDLE hProcess = OpenProcess(DesiredAccess, bInheritHandle, processId);
    if (hProcess == NULL)
        return FALSE;

    BOOL result = TerminateProcess(hProcess, exitCode);

    CloseHandle(hProcess);

    return result;
}

DWORD WinApiHelper::GetProcId(HWND windowHandle) {
    DWORD lpProcessId = NULL;
    GetWindowThreadProcessId(windowHandle, &lpProcessId);
    return lpProcessId;
}

DWORD WinApiHelper::GetProcId(const char* processName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 procEntry;
    procEntry.dwSize = sizeof(procEntry);

    Process32First(hSnap, &procEntry);
    do {
        if (!_stricmp(procEntry.szExeFile, processName)) {
            procId = procEntry.th32ProcessID;

            break;
        }
    } while (Process32Next(hSnap, &procEntry));
    CloseHandle(hSnap);
    return procId;
}

char* WinApiHelper::GetProcName(DWORD procId)
{
    char* result = new char[260];
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, procId);

    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 procEntry;
    procEntry.dwSize = sizeof(procEntry);

    Process32First(hSnap, &procEntry);
    do {
        if (procId == procEntry.th32ProcessID) {

            strcpy(result, procEntry.szExeFile);
            break;  
        }
    } while (Process32Next(hSnap, &procEntry));

    CloseHandle(hSnap);
    return result;
}

bool WinApiHelper::AddToStartup(const char* path, const char* startName)
{
    HKEY hKey;
    const char* czStartName = startName;
    long lnRes = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_WRITE,
        &hKey
    );

    bool result = false;
    if (ERROR_SUCCESS == lnRes) {
        lnRes = RegSetValueExA(
            hKey,
            czStartName,
            0,
            REG_SZ,
            (UCHAR*)path,
            strlen(path)
        );
        result = true;
    }

    RegCloseKey(hKey);
    return result;
}

WinApiHelper::ScreenshotInfo WinApiHelper::TakeScreenshot()
{
    HDC hScreenDC = GetDC(nullptr); 
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int width = GetDeviceCaps(hScreenDC, HORZRES);
    int height = GetDeviceCaps(hScreenDC, VERTRES);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);

    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
    hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));


    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    SelectObject(hMemoryDC, hBitmap);

    // Prepare DIB information
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bitmap.bmWidth;
    bmi.bmiHeader.biHeight = -bitmap.bmHeight;  
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;  
   
    unsigned char* buffer = new unsigned char[bitmap.bmWidth * bitmap.bmHeight * 3];  // 3 bytes per pixel (RGB)
    GetDIBits(hMemoryDC, hBitmap, 0, bitmap.bmHeight, buffer, &bmi, DIB_RGB_COLORS);

    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);

    for (uint32_t i = 0; i < bitmap.bmWidth * bitmap.bmHeight * 3 - 2; i+=3) {
        swap(buffer[i], buffer[i + 2]);
    }


    return ScreenshotInfo(width, height, buffer);
}

bool WinApiHelper::SetDesktopWallpers(const char* path) {
    return SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)path, SPIF_SENDCHANGE);
}

string WinApiHelper::GetDesktopWallpersPath()
{
    char result[MAX_PATH] = { '\0' };
    SystemParametersInfoA(SPI_GETDESKWALLPAPER, MAX_PATH, result, 0);
    return result;
}

bool WinApiHelper::MutexExists(const char* name)
{
    HANDLE hMutex = OpenMutex(
        MUTEX_ALL_ACCESS, 0, name);

    return hMutex;
}

HANDLE WinApiHelper::SetMutex(const char* name) {
    if (MutexExists(name))
        return NULL;

    return CreateMutexA(0, 0, name);
}

bool WinApiHelper::CloseMutex(HANDLE h) {
    return ReleaseMutex(h);
}

vector<PROCESSENTRY32> WinApiHelper::GetAllProcesses()
{
    vector<PROCESSENTRY32> result;
    
    HANDLE         hSnap;
    PROCESSENTRY32 procEntry;
    DWORD          i = 0, pe_size;

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE32, 0);
    procEntry.dwSize = sizeof(procEntry);
    Process32First(hSnap, &procEntry);

    do  {
        result.push_back(procEntry);
    } while (Process32Next(hSnap, &procEntry));

    return result;
}

vector<WinApiHelper::WindowInfo> WinApiHelper::GetAllWindowProcesses() {

    vector<WinApiHelper::WindowInfo> result;

    for (HWND hwnd = GetTopWindow(NULL); hwnd != NULL; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {

        if (!IsWindowVisible(hwnd))
            continue;

        int length = GetWindowTextLength(hwnd);
        if (length == 0)
            continue;

        char* title = new char[length + 1];
        GetWindowText(hwnd, title, length + 1);

        result.push_back(WinApiHelper::WindowInfo(hwnd, title));
    }

    return result;
}

vector<string> WinApiHelper::GetIpAddresses()
{
    vector<string> result;
    char ac[80];

#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 0);
    ::WSAStartup(wVersionRequested, &wsaData);
#endif

    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        return result;
    }
    //cout << "Host name is " << ac << "." << endl;

    hostent* phe = gethostbyname(ac);
    if (phe == 0) {
        return result;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(in_addr));
        result.push_back(inet_ntoa(addr));
        //cout << "Address " << i << ": " << inet_ntoa(addr) << endl;
    }
    return result;
}

string WinApiHelper::GetExecutablePath() {
    char buffer[MAX_PATH + 1];
    int count = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return string(buffer, (count > 0) ? count : 0);
}

