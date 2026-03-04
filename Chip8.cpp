//#define MAIN

// Header
#include "Chip8.h"

// Std
#include <cstdint>
#include <stack>

// File reading
#include <iostream>
#include <fstream>

// Timing
#include <chrono>
#include <thread>

constexpr int CHIP8_WIDTH  = 64;
constexpr int CHIP8_HEIGHT = 32;


// Constructor: Iniitializes the state when you create Chip8 chip8
Chip8::Chip8() {
    PC = 0x200; // Chip-8 programs start at 0x200
    index_register = 0;
    instr_count = 0;
    delay_timer = 0;
    sound_timer = 0;

    // Clear display and registers
    for (int i = 0; i < 2048; ++i) display[i] = false;
    for (int i = 0; i < 16; ++i) V[i] = 0;
    
    // also clear mem just to be safe
    //for (int i = 0; i < 4096; ++i) mem[i] = 0;

    initMem();
}

int Chip8::initMem() {
    for (int i = 0; i < 80; ++i) {
        mem[0x50 + i] = chars[i];
    }

    loadROM();
    return 0;
}

int Chip8:: loadROM() {
    // Open the file in binary mode and move the cursor to the end immediately
    std::ifstream file("./ROMs/IBM Logo.ch8", std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get the size of the file and reset the cursor to the beginning
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Check if the ROM fits in memory (from 0x200 to 0xFFF)
        if (size > (4096 - 0x200)) {
            std::cerr << "Error: ROM is too large for memory." << std::endl;
            return -1;
        }

        // Read the file directly into the memory array starting at 0x200
        file.read(reinterpret_cast<char*>(&mem[0x200]), size);
        file.close();

        return 0;
    }

    std::cerr << "Error: Could not open ROM file." << std::endl;
    return -1;
}

int Chip8::cycle() {
    // Fetch
    // Read instruction that PC points to
    // Each instruction is two bytes
    uint16_t opcode = (mem[PC] << 8) + mem[PC + 1];
    ++instr_count;
    
    // Increment PC
    ++PC;
    ++PC;
    
    // Decode
    uint8_t type = (opcode & 0xF000) >> 12; 
    uint8_t X    = (opcode & 0x0F00) >> 8;
    uint8_t Y    = (opcode & 0x00F0) >> 4;
    uint8_t N    = (opcode & 0x000F);
    uint8_t NN   = (opcode & 0x00FF);
    uint16_t NNN = (opcode & 0x0FFF);

    // Execute
    switch (type) {
        case 0x0:
            // 00E0 (clear screen)
            for (int i = 0; i < 2048; ++i) display[i] = 1;
            break;
        
        case 0x1:
            // 1NNN (jump)
            PC = NNN;
            break;
        
        case 0x6:
            // 6XNN (set register VX)
            V[X] = NN;
            break;

        case 0x7:
            // 7XNN (add value to register VX)
            V[X] += NN;
            break;
        
        case 0xA:
            // ANNN (set index register I)

            index_register = NNN;
            break;

        case 0xD:
            // DXYN (display/draw)
            // Draw a N-pixels tall sprite at the location specified by Reg X and Reg Y
            // Sprite drawn specified by the index register
            uint8_t x_pos = V[X];
            uint8_t y_pos = V[Y];

            // Sprite location in memory, specified by index register (index_register)
            // Iterate across character height (5)
            for (int row = 0; row < N; ++row) {
                uint8_t byte = mem[index_register + row];

                // Iterate across each bit in the byte
                for (int col = 0; col < 8; ++col) {
                    if (byte & (0x80 >> col)) {
                        // x, y is the starting point
                        // col, row offsets from there
                        // % ensures screen wrapping + no calling OOB index values
                        int px = (x_pos + col) % CHIP8_WIDTH;
                        int py = (y_pos + row) % CHIP8_HEIGHT;
                        // XOR bitflip to update screen
                        display[py * CHIP8_WIDTH + px] ^= 1;
                    }
                }
            }
            break;
    }

    // Timing
    // Original Chip-8 processors ran around 1MHz, 90's HP calculators at 4MHz
    //std::this_thread::sleep_for(std::chrono::microseconds(1));    // This will ignore how much time the instructions take
    
    
    if (instr_count % 1000 == 0) {  // Verify that CPU is cycling=
        printf("1000 Milestone!\n");
    }

    return 0;
}

#ifdef MAIN

int main() {
    Chip8 chip8;
    chip8.loadROM();

    for (int i = 0; i < 50000; i++) {
        chip8.cycle();
    }
}

#endif