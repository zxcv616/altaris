#include <iostream>
#include <winsock2.h>
#include <thread>
#include <map>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define INET_ADDRSTRLEN 16  // Manually define INET_ADDRSTRLEN

const int SERVER_PORT = 54321;
std::map<std::string, SOCKET> clients;

void handle_client(SOCKET client_socket, std::string client_id) {
    char buffer[4096];
    while (true) {
        int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "[*] Connection lost with " << client_id << std::endl;
            closesocket(client_socket);
            clients.erase(client_id);
            break;
        }
        buffer[bytesReceived] = '\0';
        std::cout << "[" << client_id << "] " << buffer << std::endl;
    }
}

void start_server() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    std::cout << "[*] Listening on port " << SERVER_PORT << std::endl;

    while (true) {
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);

        char *client_ip = inet_ntoa(client_addr.sin_addr);  // Use inet_ntoa for IP conversion
        std::string client_id = std::string(client_ip) + ":" + std::to_string(ntohs(client_addr.sin_port));

        std::cout << "[*] Accepted connection from " << client_id << std::endl;
        clients[client_id] = client_socket;
        std::thread(handle_client, client_socket, client_id).detach();
    }

    closesocket(server_socket);
    WSACleanup();
}

void send_command() {
    std::string command;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        for (const auto &pair : clients) {
            send(pair.second, command.c_str(), command.size(), 0);
        }
    }
}

int main() {
    std::thread server_thread(start_server);
    send_command();
    server_thread.join();
    return 0;
}
