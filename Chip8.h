// Chip8.h
#ifndef CHIP8
#define CHIP8

#include <cstdint>
#include <stack>

class Chip8 {
    private:
        uint8_t mem[4096];  // 12-bit addressable Memory
        uint16_t PC;    // Program Counter
        uint16_t index_register;    // Dedicated pointer register
        std::stack<std::uint16_t> stack;    // Stack

        // Timers
        uint8_t delayTimer;
        uint8_t soundTimer;
        
        // Instruction counter, if doing that method
        uint16_t instr_count;

        // V0 - VF Registers
        uint8_t V[16];
        // VF is also used as the carry flag

    public:
        Chip8();

        // Display
        bool display[64 * 32];

        // CPU
        int loadROM(char* filepath);
        int initMem();
        int cycle();
        void updateTimers();
        uint8_t readDelayTimer();
        uint8_t readSoundTimer();

        // Reading out instruction type
        uint8_t read_type();

        // Keypad input
        uint8_t keypad[16];  // Ranges from 0x00 - 0x0F

        // Charset        
        static inline constexpr uint8_t chars[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
};

#endif