#include "./Chip8/chip8.hpp"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>

// layout originale
// 1 2 3 C
// 4 5 6 D
// 7 8 9 E
// A 0 B F
int keyMap(int vkey) {
    switch (vkey) {
        case '1': return 0x1; case '2': return 0x2;
        case '3': return 0x3; case '4': return 0xC;
        case 'Q': return 0x4; case 'W': return 0x5;
        case 'E': return 0x6; case 'R': return 0xD;
        case 'A': return 0x7; case 'S': return 0x8;
        case 'D': return 0x9; case 'F': return 0xE;
        case 'Z': return 0xA; case 'X': return 0x0;
        case 'C': return 0xB; case 'V': return 0xF;
        default:  return -1;
    }
}

//per GetAsyncKeyState
const int VKEYS[16] = {
    'X','1','2','3','Q','W','E','A',
    'S','D','Z','C','4','R','F','V'
};

void updateInput(Chip8& chip8) {
    for (int i = 0; i < 16; i++) {
        chip8.keypad[i] = (GetAsyncKeyState(VKEYS[i]) & 0x8000) ? 1 : 0;    //GetAsyncKeyState da negativo se il tasto è premuto
    }
}

void render(HANDLE hConsole, CHAR_INFO* buffer,
            const std::array<uint32_t, 64 * 32>& video) {
    for (int i = 0; i < 64 * 32; i++) {
        buffer[i].Char.AsciiChar = video[i] ? '\xDB' : ' ';     // arattere pieno se pixel acceso, spazio se spento
        buffer[i].Attributes     = video[i]
            ? FOREGROUND_GREEN | FOREGROUND_INTENSITY
            : 0;
    }

    COORD bufSize  = {64, 32};
    COORD bufCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, 63, 31};
    WriteConsoleOutput(hConsole, buffer, bufSize, bufCoord, &writeRegion);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: chip8.exe <Pong1.ch8>\n";
        return 1;
    }

    //setup
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // finestra in 64x32
    SMALL_RECT windowSize = {0, 0, 63, 31};
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    COORD bufSize = {64, 32};
    SetConsoleScreenBufferSize(hConsole, bufSize);

    // no cursore
    CONSOLE_CURSOR_INFO cursorInfo = {1, FALSE};
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    CHAR_INFO screenBuffer[64 * 32] = {};   //serve per il rendering

    Chip8 chip8;

    chip8.loadRom(argv[1]);

    // ciclo principale
    using clock = std::chrono::steady_clock;
    auto lastTimer = clock::now();
    const auto cpuDelay = std::chrono::microseconds(2000); // dovrebbero essere circa 500 cicli al secondo

    while (true) {
        // si esce con ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;
        updateInput(chip8);
        chip8.emulateCycle();

        //aggiorna display
        if (chip8.drawFlag) {
            render(hConsole, screenBuffer, chip8.video);
            chip8.drawFlag = false;
        }
        auto now = clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>
                (now - lastTimer).count() >= 16) {
            lastTimer = now;
            // i timer sono gestiti da emulatecycle()
        }

        std::this_thread::sleep_for(cpuDelay);
    }

    return 0;
}