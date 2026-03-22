//#define MAIN
#define OLD

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

// Input reading (for FX0A)
#include <SDL2/SDL.h>


#define FONTSET_START_ADDRESS 0x50
#define START_ADDRESS 0x200

constexpr int CHIP8_WIDTH  = 64;
constexpr int CHIP8_HEIGHT = 32;


// Constructor: Iniitializes the state when you create Chip8 chip8
Chip8::Chip8() {
    PC = START_ADDRESS; // Chip-8 programs start at 0x200
    index_register = 0;
    instr_count = 0;
    delayTimer = 0;
    soundTimer = 0;

    // Clear display and registers
    for (int i = 0; i < 2048; ++i) display[i] = false;
    for (int i = 0; i < 16; ++i) V[i] = 0;
    for (int i = 0; i < 16; ++i) keypad[i] = 0;

    // also clear mem just to be safe
    //for (int i = 0; i < 4096; ++i) mem[i] = 0;

    initMem();
}

int Chip8::initMem() {
    for (int i = 0; i < 80; ++i) {
        mem[FONTSET_START_ADDRESS + i] = chars[i];
    }
    return 0;
}

int Chip8::loadROM(char* filepath) {
    // Open the file in binary mode and move the cursor to the end immediately
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get the size of the file and reset the cursor to the beginning
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Check if the ROM fits in memory (from 0x200 to 0xFFF)
        if (size > (4096 - START_ADDRESS)) {
            std::cerr << "Error: ROM is too large for memory." << std::endl;
            return -1;
        }

        // Read the file directly into the memory array starting at 0x200
        file.read(reinterpret_cast<char*>(&mem[START_ADDRESS]), size);
        file.close();

        return 0;
    }

    std::cerr << "Error: Could not open ROM file." << std::endl;
    return -1;
}

void Chip8::updateTimers() {
    if (delayTimer > 0) {
        --delayTimer;
    }

    if (soundTimer > 0) {
        --soundTimer;
        // Trigger beep sound while > 0
    }
}

uint8_t Chip8::readDelayTimer() {
    return delayTimer;
}

uint8_t Chip8::readSoundTimer() {
    return soundTimer;
}

uint8_t Chip8::read_type() {
    return (mem[PC - 2] & 0xF0) >> 2;   // Decrement by 2 because this will be run after current instruction has been incremented
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
        {
            switch (opcode) {
                case 0x00E0:
                    // 00E0: CLS
                    // Clear the display
                    for (int i = 0; i < 2048; ++i) display[i] = 0;
                    break;
                
                case 0x00EE:
                    // 00EE: RET
                    // Return from subroutine

                    // Pop the stack and set the PC to the popped value
                    PC = stack.top();
                    stack.pop();
                    break;
            }
            break;
        }
        
        case 0x1:
        {
            // 1NNN: JP addr
            // Jump to location at NNN
            PC = NNN;
            break;
        }

        case 0x2:
        {
            // 2NNN: CALL addr
            // Call Subroutine at NNN
            stack.push(PC);
            PC = NNN;
            break;
        }

        case 0x3:
        {
            // 3XNN: SE Vx, byte
            // Skip next instruction if V[X] == NN (used to branch)
            if (V[X] == NN) {
                ++PC;
                ++PC;
            }
            break;
        }
        
        case 0x4:
        {
            // 4XNN: SNE Vx, byte
            // Skip next instruction if V[X] != NN (used to branch)
            if (V[X] != NN) {
                ++PC;
                ++PC;
            }
            break;
        }

        case 0x5:
        {
            // 5XY0: SE Vx, Vy
            // Skip next instruction if V[X] == V[Y] (used to branch)
            if (V[X] == V[Y]) {
                ++PC;
                ++PC;
            }
            break;
        }
        
        case 0x6:
        {
            // 6XNN: LD Vx, byte
            // Set register VX = NN
            V[X] = NN;
            break;
        }

        case 0x7:
        {
            // 7XNN: ADD Vx, byte
            // Add NN to register VX
            V[X] += NN;
            break;
        }
        
        case 0x8:
        {
            switch (N) {
                case 0x0:
                {
                    // 8XY0: LD Vx, Vy
                    // Set V[X] = V[Y]
                    V[X] = V[Y];
                } break;
                case 0x1:
                {
                    // 8XY1: OR Vx, Vy
                    // Perform bitwise OR on V[X] and V[Y] and store output in V[X]
                    V[X] |= V[Y];

                    // Also reset V[F]
                    V[0xF] = 0;
                } break;
                case 0x2:
                {
                    // 8XY2: AND Vx, Vy
                    // Perform bitwise AND on V[X] and V[Y] and store output in V[X]
                    V[X] &= V[Y];

                    // Also reset V[F]
                    V[0xF] = 0;
                } break;
                case 0x3:
                {
                    // 8XY3: XOR Vx, Vy
                    // Perform bitwise XOR on V[X] and V[Y] and store output in V[X]
                    V[X] ^= V[Y];

                    // Also reset V[F]
                    V[0xF] = 0;
                } break;
                case 0x4:
                {
                    // 8XY4: ADD Vx, Vy
                    // Add V[X] with V[Y] and store output in V[X]. Set V[F] as the carry
                    
                    // First, store what the carry value should be
                    // We do this separately because V[X] could possibly be V[F]
                    uint8_t temp;
                    if ((int) V[X] + (int) V[Y] > 255) {
                        temp = 1;
                    } else {
                        temp = 0;
                    }

                    V[X] += V[Y];
                    // Set the carry flag
                    V[0xF] = temp;
                } break;
                case 0x5:
                {
                    // 8XY5: SUB Vx, Vy
                    // Subtract V[X] with V[Y] and store output in V[X]. Set V[F] as NOT borrow

                    // First, store what the carry value should be
                    // We do this separately because V[X] could possibly be V[F]
                    uint8_t temp;
                    if (V[X] >= V[Y]) {
                        temp = 1;
                    } else {
                        temp = 0;
                    }

                    V[X] = V[X] - V[Y];
                    // Set the carry flag
                    V[0xF] = temp;
                } break;
                case 0x6:
                {
                    // 8XY6: SHR Vx {, Vy}
                    // Shift V[X] right once

                    // OPTIONAL: Some systems first set Vx to Vy before shifting. Make configurable later
#ifdef OLD
                    V[X] = V[Y];
#endif
                    
                    // First, store what the carry value should be
                    // We do this separately because V[X] could possibly be V[F]
                    uint8_t temp;
                    temp = V[X] & 0b1; // Get the last bit (shifted out)

                    V[X] = V[X] >> 1;

                    V[0xF] = temp;
                } break;
                case 0x7:
                {
                    // 8XY7: SUBN Vx, Vy
                    // Subtract V[Y] with V[X] and store output in V[X]. Set V[F] as NOT borrow

                    // First, store what the carry value should be
                    // We do this separately because V[X] could possibly be V[F]
                    uint8_t temp;
                    if (V[Y] >= V[X]) {
                        temp = 1;
                    } else {
                        temp = 0;
                    }

                    V[X] = V[Y] - V[X];

                    V[0xF] = temp;
                } break;
                case 0xE:
                {
                    // 8XYE: SHL Vx {, Vy}
                    // Shift V[X] left once

                    // OPTIONAL: Some systems first set Vx to Vy before shifting. Make configurable later
#ifdef OLD
                    V[X] = V[Y];
#endif
                    
                    // First, store what the carry value should be
                    // We do this separately because V[X] could possibly be V[F]
                    uint8_t temp;
                    temp = V[X] >> 7; // Get the last bit (shifted out)

                    V[X] = V[X] << 1;

                    V[0xF] = temp;
                } break;
            }
            break;
        }
        
        case 0x9:
        {
            // 9XY0: SNE Vx, Vy
            // Skip next instruction if V[X] != V[Y]
            if (V[X] != V[Y]) {
                ++PC;
                ++PC;
            }
            break;
        }
        
        case 0xA:
        {
            // ANNN: LD I, addr
            // Set index register I = NNN

            index_register = NNN;
            break;
        }

        case 0xB:
        {
            // BNNN: JP V0, addr
            // Jump to instruction at address NNN, plus offset specified by V0
            PC = NNN + V[0];
            // Newer Chip8 ISAs use BXNN instead, using X to specify the offset register
            // Add configurability to specify this offset
            // PC == NN + V[X];
            break;
        }

        case 0xC:
        {
            // CXNN: RND Vx, byte
            // Generate random number, AND with NN, set to V[X}]
            uint8_t randNum = rand() % 0xFF;
            V[X] = randNum & NN;
            break;
        }

        case 0xD:
        {
            // DXYN: DRW Vx, Vy, nibble
            // Display/draw n-byte sprite from memory location I at V[X], V[Y]
            // NOTE: here, screen wrapping is used, but most programs will clip the sprite instead of wrap
            uint8_t x_pos = V[X] & 63;  // AND with 0b111111, effectively modulo 64
            uint8_t y_pos = V[Y] & 31;  // AND with 0b11111, effectively modulo 32
            V[0xF] = 0;

            // Iterate across character height (5)
            for (int row = 0; row < N; ++row) {
                uint8_t byte = mem[index_register + row];

                // Iterate across each bit in the byte
                for (int col = 0; col < 8; ++col) {
                    if (byte & (0x80 >> col)) {
                        // x, y is the starting point
                        // col, row offsets from there
                        // % ensures screen wrapping + no calling OOB index values
                        int px = x_pos + col;
                        int py = y_pos + row;

                        if (px < 64 && py < 32) {
                            // XOR bitflip to update screen
                            display[py * CHIP8_WIDTH + px] ^= 1;
                            // Collision detection
                            if ((display[py * CHIP8_WIDTH + px]) == 0) {
                                V[0xF] = 1;
                            }
                        }
                    }
                }
            }
            break;
        }

        case 0xE:
        {
            switch (NN) {
                case 0x9E:
                {
                    // EX9E: SKP Vx
                    // Skip if key press matches V[X]
                    if (keypad[V[X]]) {
                        ++PC;
                        ++PC;
                    }
                } break;

                case 0xA1:
                {
                    // EXA1: 
                    // Skip if key press doesn't match V[X]
                    if (!keypad[V[X]]) {
                        ++PC;
                        ++PC;
                    }
                } break;
            }
            break;
        }

        case 0xF:
        {
            switch (NN) {
                case 0x07:
                {
                    // FX07: LD Vx, DT
                    // Set Register VX = delayTimer
                    V[X] = delayTimer;
                } break;
                
                case 0x0A:
                {
                    // FX0A: LD Vx, key
                    // Stop execution, wait for key press, and store value into Vx

                    bool reset = true;
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i]) {
                            V[X] = i;
                            reset = false;
                            break;
                        }
                    }

                    if (reset) {
                        --PC;
                        --PC;
                    }
                } break;

                case 0x15:
                {
                    // FX15: LD DT, Vx
                    // Set delay timer to Register VX
                    delayTimer = V[X];
                } break;

                case 0x18:
                {
                    // FX18: LD ST, Vx
                    // Set sound timer to Register VX
                    soundTimer = V[X];
                } break;

                case 0x1E:
                {
                    // FX1E: ADD I, Vx
                    // Add Vx to index register
                    index_register += V[X];
                } break;
                
                case 0x29:
                {
                    // FX29: LD F, Vx
                    // Set index register to location of sprite corresponding to register VX

                    // Hex sprites start at 0x50
                    // Each sprite is 5 bytes, so must offset by V[X] (sprite value) x 5 (mem unit)
                    index_register = FONTSET_START_ADDRESS + V[X] * 5;
                } break;

                case 0x33:
                {
                    // FX33: LD B, Vx
                    // Store BCD representation of V[X] in memory locations I, I+1, and I+2
                    // I: Hundreds digit
                    mem[index_register] = V[X] / 100;
                    // I+1: Tens digit
                    mem[index_register + 1] = (V[X] % 100) / 10;
                    // I+2: Ones digit
                    mem[index_register + 2] = (V[X] % 10);
                } break;

                case 0x55:
                {
                    // FX55: LD [I], Vx
                    // Store V[0]-V[F] in memory, starting at I
#ifdef OLD
                    for (int i = 0; i <= X; ++i) {
                        mem[index_register++] = V[i];
                    }
#else
                    for (int i = 0; i <= X; ++i) {
                        mem[index_register + i] = V[i];
                    }
#endif
                } break;

                case 0x65:
                {
                    // FX65: LD Vx, [I]
                    // Read registers V[0]-V[F] from memory, starting at I
#ifdef OLD
                    for (int i = 0; i <= X; ++i) {
                        V[i] = mem[index_register++];
                    }
#else
                    for (int i = 0; i <= X; ++i) {
                        V[i] = mem[index_register + i];
                    }
#endif
                } break;
            }
            break;
        }
    }

    
    if (instr_count % 1000 == 0) {  // Verify that CPU is cycling=
        //printf("1000 Milestone!\n");
        ;
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