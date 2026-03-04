// For outputting to the terminal
#include <iostream>

// Std int
#include <cstdint>
// For reading/writing into files
#include <fstream>

#include <stack>

// For timing
#include <chrono>
#include <thread>


// Memory: CHIP-8 has direct access to up to 4 kilobytes of RAM
// Display: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP) monochrome, ie. black or white
// Program Counter: often called just “PC”, which points at the current instruction in memory
// One 16-bit index register: called “I” which is used to point at locations in memory
// Stack for 16-bit addresses: used to call subroutines/functions and return from them
// 8-bit delay timer: decremented at a rate of 60 Hz (60 times per second) until it reaches 0
// 8-bit sound timer: functions like the delay timer, but which also gives off a beeping sound as long as it’s not 0
// 16 8-bit general-purpose variable registers: numbered 0 through F hexadecimal, called V0 through VF
// VF is also used as a flag register; many instructions will set it to either 1 or 0 based on some rule, for example using it as a carry flag


int main()
{
    std::cout << "Hello World!" << std::endl;

    // Memory
    // The memory should be 4 kB (4 kilobytes, ie. 4096 bytes) large. 
    // CHIP-8’s index register and program counter can only address 12 bits (conveniently), which is 4096 addresses.
    // -- 4096 elements
    // -- each element (byte) is 8 bits

    uint8_t mem[4096];

    // The first CHIP-8 interpreter (on the COSMAC VIP computer) was also located in RAM, from address 000 to 1FF. 
    // It would expect a CHIP-8 program to be loaded into memory after it, starting at address 200 (512 in decimal). 
    // Although modern interpreters are not in the same memory space, you should do the same to be able to run the old programs; 
    // you can just leave the initial space empty, except for the font.
    // -- So leave addresses 000-1FF (0-511) empty, start writing at 0x200

    // Program Counter: points to location in memory, where program will execute
    uint16_t PC;    // In reality, only 12 bits long, enough to address from 0x000 - 0xFFF

    // Index Register: similar principle of pointing to location in memory
    uint16_t index_register;

    // General-purpose registers
    uint8_t V0;
    uint8_t V1;
    uint8_t V2;
    uint8_t V3;
    uint8_t V4;
    uint8_t V5;
    uint8_t V6;
    uint8_t V7;
    uint8_t V8;
    uint8_t V9;
    uint8_t VA;
    uint8_t VB;
    uint8_t VC;
    uint8_t VD;
    uint8_t VE;
    uint8_t VF;

    // Font
    // The CHIP-8 emulator should have a built-in font, with sprite data representing the hexadecimal numbers from 0 through F. 
    // Each font character should be 4 pixels wide by 5 pixels tall. These font sprites are drawn just like regular sprites (see below).
    // You should store the font data in memory, because games will draw these characters like regular sprites: 
    // They set the index register I to the character’s memory location and then draw it. 
    // There’s a special instruction for setting I to a character’s address, so you can choose where to put it. 
    // Anywhere in the first 512 bytes (000–1FF) is fine. 
    // For some reason, it’s become popular to put it at 050–09F (80-159), so you can follow that convention if you want.

    // Define a 2D array holding the character data
    uint8_t chars[16][5] = {
        {0xF0, 0x90, 0x90, 0x90, 0xF0}, // 0
        {0x20, 0x60, 0x20, 0x20, 0x70}, // 1
        {0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
        {0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
        {0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
        {0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
        {0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
        {0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
        {0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
        {0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
        {0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
        {0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
        {0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
        {0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
        {0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
        {0xF0, 0x80, 0xF0, 0x80, 0x80}  // F
    };

    // Fill the memory space starting at mem[80]
    int index = 80;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 5; j++) {
            mem[index++] = chars[i][j];
        }
    }
    // Optional: Print out the memory to verify it
    //for (int i = 80; i < 160; ++i) {
    //    std::cout << "mem[" << i << "] = 0x" << std::hex << (int)mem[i] << std::endl;
    //}


    // Display
    // 64 x 32 pixels
    // 60 hz refresh rate
    // Probably better to only update the display when an instruction modifies display elements
    // I think I would have to learn OpenGL to render this
    bool display[64 * 32];

    // Stack
    std::stack<std::uint16_t> stack;
    // usually old programs had at most 2 stack elements
    // Can keep it unlimited for now

    // Timers
    // 1 byte and decrements at 60 hz (while > 1)
    // Decrements and rate would be incorporated in the main loop, at least that's what I think
    uint8_t delay_timer;
    uint8_t sound_timer;


    // Keypad
    // input grid, 4x4, from 0-F
    // 1 2 3 C
    // 4 5 6 D
    // 7 8 9 E
    // A 0 B F
    // I want the following map of keys to be mapped to the above
    // 1 2 3 4
    // Q W E R
    // A S D F
    // Z X C V

    // Instruction counter, if doing that method
    uint16_t instr_count = 0;


    // Main loop

    while (1) {
        // Fetch
        // Read instruction that PC points to
        // Each instruction is two bytes
        uint16_t instr = ((uint32_t) mem[PC] << 8) + mem[PC + 1];
        ++instr_count;
        
        // Increment PC
        ++PC;
        ++PC;   // Do it twice, to go to next two bytes
        

        // Decode
        // Instruction is 16 bits (two bytes)
        // [      ][                      ]
        // First nibble: instruction type
        // Last three nibles: arguments
        
        // [   Type  ][  X  ][  Y  ][  N  ]
        // For instr1:
        // X: Register X
        // Y: Register Y
        // N: 4-bit number

        // [   Type   ][   X   ][    NN   ]
        // For instr2:
        // X: Register X
        // NN: 8-bit immediate number

        // [   Type   ][        NNN       ]
        // NNN: 12-bit immediate memory address

        // Instructions
        // 00E0 (clear screen)
        // 1NNN (jump)
        // 6XNN (set register VX)
        // 7XNN (add value to register VX)
        // ANNN (set index register I)
        // DXYN (display/draw)
        
        // Execute

        uint8_t instr_type = instr / 4096;  // First nibble
        uint8_t X;
        uint8_t Y;
        uint8_t N;
        uint8_t NN;
        uint16_t NNN;


        switch (instr_type) {
            case 0x0:
                X = (instr % 4096) / 256;
                Y = (instr % 256) / 16;
                N = instr % 16;
                // 00E0 (clear screen)
                // Iterate across display matrix and set all pixels
                break;
            
            case 0x1:
                NNN = instr % 4096;
                // 1NNN (jump)
                PC = NNN;
                break;
            
            case 0x6:
                X = (instr % 4096) / 256;
                NN = instr % 256;
                // 6XNN (set register VX)
                if (X == 0x0) {
                    V0 = NN;
                } else if (X == 0x1) {
                    V1 = NN;
                } else if (X == 0x2) {
                    V2 = NN;
                } else if (X == 0x3) {
                    V3 = NN;
                } else if (X == 0x4) {
                    V4 = NN;
                } else if (X == 0x5) {
                    V5 = NN;
                } else if (X == 0x6) {
                    V6 = NN;
                } else if (X == 0x7) {
                    V7 = NN;
                } else if (X == 0x8) {
                    V8 = NN;
                } else if (X == 0x9) {
                    V9 = NN;
                } else if (X == 0xA) {
                    VA = NN;
                } else if (X == 0xB) {
                    VB = NN;
                } else if (X == 0xC) {
                    VC = NN;
                } else if (X == 0xD) {
                    VD = NN;
                } else if (X == 0xE) {
                    VE = NN;
                } else if (X == 0xF) {
                    VF = NN;
                }
                break;

            case 0x7:
                X = (instr % 4096) / 256;
                NN = instr % 256;
                // 7XNN (add value to register VX)
                if (X == 0x0) {
                    V0 += NN;
                } else if (X == 0x1) {
                    V1 += NN;
                } else if (X == 0x2) {
                    V2 += NN;
                } else if (X == 0x3) {
                    V3 += NN;
                } else if (X == 0x4) {
                    V4 += NN;
                } else if (X == 0x5) {
                    V5 += NN;
                } else if (X == 0x6) {
                    V6 += NN;
                } else if (X == 0x7) {
                    V7 += NN;
                } else if (X == 0x8) {
                    V8 += NN;
                } else if (X == 0x9) {
                    V9 += NN;
                } else if (X == 0xA) {
                    VA += NN;
                } else if (X == 0xB) {
                    VB += NN;
                } else if (X == 0xC) {
                    VC += NN;
                } else if (X == 0xD) {
                    VD += NN;
                } else if (X == 0xE) {
                    VE += NN;
                } else if (X == 0xF) {
                    VF += NN;
                }
                break;
            
            case 0xA:
                NNN = instr % 4096;
                // ANNN (set index register I)

                index_register = NNN;
                break;

            case 0xD:
                X = (instr % 4096) / 256;
                Y = (instr % 256) / 16;
                N = instr % 16;
                // DXYN (display/draw)
        }

        

        // Timing
        // Original Chip-8 processors ran around 1MHz, 90's HP calculators at 4MHz
        std::this_thread::sleep_for(std::chrono::microseconds(1));    // This will ignore how much time the instructions take
        
        // Guide recommends around 700 instructions per second
        // Maybe I just have an external counter for how many instructions have been executed?
        // Wait 1 second (or what's left in the second) after this, to start next frame

        
        if (instr_count % 1000 == 0) {  // Verify that CPU is cycling=
            printf("1000 Milestone!\n");
        }

        if (instr_count == 50000) { // Terminate program after 50000 instructions
            break;
        }

    }


    return 0;
}