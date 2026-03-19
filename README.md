# CHIP-8 Emulator

CHIP-8 scritto in C++ senza dipendenze esterne.
Il rendering avviene direttamente nel terminale Windows tramite Console API.

## Requisiti

- Windows
- Microsoft Visual Studio Build Tools (cl.exe)

## Compilazione

**Developer PowerShell for VS**:
```cmd
cl /std:c++17 /EHsc src\main.cpp src\Chip8\chip8.cpp /Fe:src\chip8.exe user32.lib
```

## Utilizzo
```cmd
src\chip8.exe rom\NomeRom.ch8
```

## Tastiera

Il keypad originale del CHIP-8 è stato mappato:

| CHIP-8  | Tastiera|
|---------|---------|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

## Struttura 
```
chip-8/
├── src/
│   ├── main.cpp          # loop principale, rendering, input
│   └── Chip8/
│       ├── chip8.hpp     # definizione della classe
│       └── chip8.cpp     # implementazione CPU e memoria
└── rom/                  # cartella per le ROM
```

## Opcode

Tutte le 35 opcode standard del CHIP-8 sono implementate, incluse:
- Aritmetica e logica (0x8xy_)
- Salti e subroutine (0x1000, 0x2000, 0x00EE)
- Input da tastiera (0xEx9E, 0xExA1)
- Rendering sprite con rilevamento collisioni (0xDxyn)
- Timer delay e sound (0xFx07, 0xFx15, 0xFx18)
