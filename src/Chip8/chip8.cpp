#include "chip8.hpp"
#include <fstream>
#include <stdexcept>

const uint8_t FONT_SET[80] = {
    0xF0,0x90,0x90,0x90,0xF0, // 0
    0x20,0x60,0x20,0x20,0x70, // 1
    0xF0,0x10,0xF0,0x80,0xF0, // 2
    0xF0,0x10,0xF0,0x10,0xF0, // 3
    0x90,0x90,0xF0,0x10,0x10, // 4
    0xF0,0x80,0xF0,0x10,0xF0, // 5
    0xF0,0x80,0xF0,0x90,0xF0, // 6
    0xF0,0x10,0x20,0x40,0x40, // 7
    0xF0,0x90,0xF0,0x90,0xF0, // 8
    0xF0,0x90,0xF0,0x10,0xF0, // 9
    0xF0,0x90,0xF0,0x90,0x90, // A
    0xE0,0x90,0xE0,0x90,0xE0, // B
    0xF0,0x80,0x80,0x80,0xF0, // C
    0xE0,0x90,0x90,0x90,0xE0, // D
    0xF0,0x80,0xF0,0x80,0xF0, // E
    0xF0,0x80,0xF0,0x80,0x80  // F
};

Chip8::Chip8(){
    srand(time(nullptr));
    pc = 0x200;   //partenza per le rom
    opcode = 0;
    index = 0;
    sp = 0;

    for(int i = 0; i<80; i++){
        memory[0x050 + i] = FONT_SET[i];
    }

};

void Chip8::loadRom(const std::string& filename){

    std::ifstream file(filename, std::ios::binary | std::ios::ate);  //apro il file in binario e metto il cursore alla fine

    if(!file.is_open())
        throw std::runtime_error("errore"+filename);

    std::streamsize size = file.tellg();    //tellg() restituisce la posizione del cursore

    if(size > static_cast<std::streamsize>(memory.size() - 0x200))  //calcolo da 0x200 in poi perchè è memoria riservata al font + sistema
        throw std::runtime_error("troppo grande");

    file.seekg(0, std::ios::beg);   //sposto il cursore all'inizio del file
    file.read(reinterpret_cast<char*>(memory.data() + 0x200), size);    //copia la rom da 0x200

    //memory.data() puntatore all'inizio della ram, +0x200 le rom partono da qui, cast il puntatore a char* perchè read lavora con char
    //size per sapere quanti byte leggere
}

void Chip8::emulateCycle(){
    //questa funzione esegue un'istruzione della rom, ogni ciclo fa fetch, decode, execute, update timer

    //inizio con fetch
    opcode = (memory[pc] << 8) | memory[pc+1];  //memory pc è il byte alto e memory pc+1 è quello basso e li unisco
    pc+=2;
    // DECODE & EXECUTE: switcha sul nibble più alto
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: // CLS. pulisce lo schermo
                    video.fill(0);
                    drawFlag = true;
                    break;
                case 0x00EE: // RET. ritorna da subroutine
                    pc = stack[--sp];
                    break;
            }
            break;

        case 0x1000: // JP addr
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: // CALL addr
            stack[sp++] = pc;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000: // SE Vx, byte
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 2;
            break;

        case 0x6000: // LD Vx, byte
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;

        case 0x7000: // ADD Vx, byte
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;

        case 0xA000: // LD I, addr
            index = opcode & 0x0FFF;
            break;

        case 0xD000: { // DRW Vx, Vy, nibble !!!!!!!
            uint8_t x   = registers[(opcode & 0x0F00) >> 8] % 64;
            uint8_t y   = registers[(opcode & 0x00F0) >> 4] % 32;
            uint8_t h   = opcode & 0x000F;
            registers[0xF] = 0;

            for (int row = 0; row < h; row++) {
                uint8_t sprite = memory[index + row];
                for (int col = 0; col < 8; col++) {
                    if (sprite & (0x80 >> col)) {
                        size_t idx = (y + row) * 64 + (x + col);
                        if (video[idx]) registers[0xF] = 1; // collisione
                        video[idx] ^= 0xFFFFFFFF;
                    }
                }
            }
            drawFlag = true;
            break;
        }

        case 0x4000: // SNE Vx, byte. salta se Vx != byte
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 2;
            break;

        case 0x5000: // SE Vx, Vy. salta se Vx == Vy
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4])
                pc += 2;
            break;

        case 0x8000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            switch (opcode & 0x000F) {
                case 0x0: registers[x]  = registers[y]; break;          // LD Vx, Vy
                case 0x1: registers[x] |= registers[y]; break;          // OR
                case 0x2: registers[x] &= registers[y]; break;          // AND
                case 0x3: registers[x] ^= registers[y]; break;          // XOR
                case 0x4: {                                              // ADD con carry
                    uint16_t sum = registers[x] + registers[y];
                    registers[0xF] = sum > 0xFF ? 1 : 0;
                    registers[x] = sum & 0xFF;
                    break;
                }
                case 0x5:                                                // SUB
                    registers[0xF] = registers[x] > registers[y] ? 1 : 0;
                    registers[x] -= registers[y];
                    break;
                case 0x6:                                                // SHR
                    registers[0xF] = registers[x] & 0x1;
                    registers[x] >>= 1;
                    break;
                case 0x7:                                                // SUBN
                    registers[0xF] = registers[y] > registers[x] ? 1 : 0;
                    registers[x] = registers[y] - registers[x];
                    break;
                case 0xE:                                                // SHL
                    registers[0xF] = (registers[x] >> 7) & 0x1;
                    registers[x] <<= 1;
                    break;
            }
            break;
        }

        case 0x9000: // SNE Vx, Vy. salta se Vx != Vy
            if (registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4])
                pc += 2;
            break;

        case 0xB000: // JP V0, addr
            pc = (opcode & 0x0FFF) + registers[0];
            break;

        case 0xC000: // RND Vx, byte. numero random AND byte
            registers[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            break;

        case 0xE000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x9E: // SKP Vx. salta se tasto Vx premuto
                    if (keypad[registers[x]]) pc += 2;
                    break;
                case 0xA1: // SKNP Vx. salta se tasto Vx NON premuto
                    if (!keypad[registers[x]]) pc += 2;
                    break;
            }
            break;
        }

        case 0xF000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x07: // LD Vx, DT
                    registers[x] = delayTimer;
                    break;
                case 0x0A: { // LD Vx, K. aspetta un tasto
                    bool pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (keypad[i]) { registers[x] = i; pressed = true; break; }
                    }
                    if (!pressed) pc -= 2; // ripete l'istruzione finché non viene premuto
                    break;
                }
                case 0x15: // LD DT, Vx
                    delayTimer = registers[x];
                    break;
                case 0x18: // LD ST, Vx
                    soundTimer = registers[x];
                    break;
                case 0x1E: // ADD I, Vx
                    index += registers[x];
                    break;
                case 0x29: // LD F, Vx. punta al font del digit Vx
                    index = 0x050 + (registers[x] * 5);
                    break;
                case 0x33: // LD B, Vx  BCD
                    memory[index]     = registers[x] / 100;
                    memory[index + 1] = (registers[x] / 10) % 10;
                    memory[index + 2] = registers[x] % 10;
                    break;
                case 0x55: // LD [I], Vx. salva registri in memoria
                    for (int i = 0; i <= x; i++)
                        memory[index + i] = registers[i];
                    break;
                case 0x65: // LD Vx, [I]. carica registri da memoria
                    for (int i = 0; i <= x; i++)
                        registers[i] = memory[index + i];
                    break;
            }
            break;
        }

    }

    // decrementati a 60 Hz
    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) {
        // bip se audio
        --soundTimer;
    }

    
    


}

