#pragma once

#include <cstdint>
#include <array>
#include <string>

class Chip8{
    //hardware
    private:
    std::array<uint8_t, 4096> memory{};
    std::array<uint8_t, 16> registers{};

    uint16_t index{}; //indirizzi di memoria
    uint16_t pc{};  //program counter

    std::array<uint16_t, 16> stack{};
    uint8_t sp{}; //stackpointer
    
    uint8_t delayTimer{};
    uint8_t soundTimer{};

    uint16_t opcode{};

    public:

    Chip8();

    void loadRom(const std::string& filename);
    void emulateCycle();

    std::array<uint32_t, 64*32> video{};

    std::array<uint8_t, 16> keypad;
    bool drawFlag{};
};