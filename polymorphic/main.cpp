#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>

using namespace std;

void xorEncryptDecrypt(char *data, int len, char key) {
    for (int i = 0; i < len; i++) {
        data[i] ^= key; // XOR en/de
    }
}

void payload() {
    cout << "Code executed" << endl;
    // continue
}

void decryptorStub() {
    char key = rand() % 256; // create random xor key
    xorEncryptDecrypt((char *)payload, sizeof(&payload), key);
    payload();
}

bool antiDebug() {
    if (IsDebuggerPresent()) {
        cout << "Debugger present" << endl;
        return true; }
        return false;
}

int main() {
    srand(time(0));

    if (antiDebug()) {
        cout << "Exiting due to debugging environment" << endl;
        return 0;
    }

    decryptorStub();
    return 0;
}
