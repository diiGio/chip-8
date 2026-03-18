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


    
    //DA FARE DECODE ED EXECUTE (MI IMPAZZISCO, CHE IDEA DI MERDA)


}

