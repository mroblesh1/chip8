#include "Chip8.h"
#include "Platform.h"

#include <SDL2/SDL.h>


constexpr int CHIP8_WIDTH  = 64;
constexpr int CHIP8_HEIGHT = 32;
constexpr int SCALE        = 10;

int main(int argc, char** argv) {
    Chip8 chip8;
    Platform platform("CHIP-8 Emulator", CHIP8_WIDTH * SCALE, CHIP8_HEIGHT * SCALE);

    bool quit = false;

    int i = 0;  // just for program termination

    while (!quit) {
        // 1. Handle Input
        // quit = platform.ProcessInput(chip8.keypad);
        if (i > 300) {
            break;
        }

        // 2. Run CPU Cycles
        // For a smooth experience, run ~10 cycles per frame 
        // instead of sleeping for 1 microsecond inside the class
        for(int i = 0; i < 10; i++) {
            chip8.cycle();
        }

        // 3. Update Graphics
        // We pass the address of chip8's internal display array
        platform.Update(chip8.display);

        // 4. Cap Frame Rate (~60 FPS)
        SDL_Delay(16); 

        i++;
    }

    return 0;
}