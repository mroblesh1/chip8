#define DEBUG

#include "Chip8.h"
#include "Platform.h"

#include <SDL2/SDL.h>


// Empscripten (for web rendering)
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

constexpr int CHIP8_WIDTH  = 64;
constexpr int CHIP8_HEIGHT = 32;
constexpr int SCALE        = 14;


Chip8 chip8;
Platform platform("CHIP-8 Emulator", CHIP8_WIDTH * SCALE, CHIP8_HEIGHT * SCALE);

bool quit = false;

void main_loop();


int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Please enter one ROM\n");
        return -1;
    }

    char* ROM = argv[1];

    chip8.loadROM(ROM);

    #ifdef __EMSCRIPTEN__
    // Loop for web environemnt
    emscripten_set_main_loop(main_loop, 0, true);
    // 0 = use browser's requestAnimationFrame (60fps target)
    // true = simulate infinite loop
    #else

    // Loop for local environment
    while (!quit) {
        main_loop();
    }

    #endif

    return 0;
}


void main_loop() {
    // 1. Handle Input
    quit = platform.ProcessInput(chip8.keypad);

    // 2. Run CPU Cycles
    for(int i = 0; i < 12; i++) {
        // 12 instructions per frame @ 60 fps, or 720 instructions per second (720 ips)
        // Guide recommends 700 ips as a starting point, so this is good
        chip8.cycle();

        // Only 1 draw instruction per frame
        if (chip8.read_type() == 0xD) {
            break;
        }
    }

    // 3. Update timers
    chip8.updateTimers();

    // Audio Logic
    if (chip8.readSoundTimer() > 0) {
        platform.PlaySound();
    } else {
        platform.StopSound();
    }

    // 4. Update Graphics
    // We pass the address of chip8's internal display array
    platform.Update(chip8.display);

    // 5. Cap Frame Rate (~60 FPS)
    SDL_Delay(16); 

}