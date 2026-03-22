//#define MAIN

#include "Platform.h"

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <cstdint>
#include <cstring>


constexpr int CHIP8_WIDTH  = 64;
constexpr int CHIP8_HEIGHT = 32;
constexpr int SCALE        = 14;


// Audio callback (defines the audio to be played, square wave)
void AudioCallback(void* userdata, uint8_t* stream, int len) {
    static uint32_t phase = 0;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / sizeof(int16_t);

    for (int i = 0; i < samples; ++i) {
        // 44100 (Sample Rate) / 440 (Frequency) = ~100 samples per cycle
        // If phase < 50, high volume. If phase > 50, low volume.
        buffer[i] = (phase % 100 < 50) ? 3000 : -3000; 
        phase++;
    }
}

Platform::Platform(char const* title, int windowWidth, int windowHeight) {
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);   // Init SDL video and audio subsystem

    window = SDL_CreateWindow(
        "CHIP-8 Renderer",
        SDL_WINDOWPOS_CENTERED, // Center window
        SDL_WINDOWPOS_CENTERED, // Center window
        windowWidth,    // Window width
        windowHeight,   // Window height
        SDL_WINDOW_OPENGL       // Enable OpenGL rendering
    );

    // Allow OpenGL to draw in the SDL window
    glContext = SDL_GL_CreateContext(window);

    // OpenGL 2D setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

    // Reset OpenGL transformations
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Define the AudioSpecs
    SDL_AudioSpec wanted, obtained;
    SDL_zero(wanted);
    wanted.freq = 44100;    // # of snapshots of sound per second. CD quality
    wanted.format = AUDIO_S16SYS;   // 16-bit int for sound
    wanted.channels = 1;
    wanted.samples = 512;
    wanted.callback = AudioCallback;    // "Don't call us, we'll call you"
    // With Callback, SDL pauses the program to request more audio data
    
    // Open the audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, 0);
}

Platform::~Platform() {
    SDL_CloseAudioDevice(audioDevice); // Clean up
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::PlaySound() {
    SDL_PauseAudioDevice(audioDevice, 0); // 0 means "unpause"
}

void Platform::StopSound() {
    SDL_PauseAudioDevice(audioDevice, 1); // 1 means "pause"
}

void Platform::Update(void const* buffer) {
    // Take in an input buffer and display onto the screen
    auto* display = (const uint8_t*)buffer;

    glClear(GL_COLOR_BUFFER_BIT);   // Clear screen
    glColor3f(1.f, 1.f, 1.f);       // Draw color = white
    glBegin(GL_QUADS);              // Draw pixels as quads

    for (int y = 0; y < CHIP8_HEIGHT; ++y) {    // Iterate across display rows
        for (int x = 0; x < CHIP8_WIDTH; ++x) { // Iterate across dislay columns
            if (display[y * CHIP8_WIDTH + x]) { // Only draw active pixels
                // Scale selected pixel distance
                float px = x * SCALE;
                float py = y * SCALE;

                // Draw 10x10 square at location
                glVertex2f(px,           py);
                glVertex2f(px + SCALE,   py);
                glVertex2f(px + SCALE,   py + SCALE);
                glVertex2f(px,           py + SCALE);
            }
        }
    }
    glEnd();
    // Double buffering
    // Draw on back buffer while still displaying front buffer, buffers swap
    SDL_GL_SwapWindow(window);  // Swap buffers
}


bool Platform::ProcessInput(uint8_t* keypad) {
    bool quit = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            {
                quit = true;
                break;
            } break;

            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    {
                        quit = true;
                        break;
                    } break;
                    case SDLK_x:
                    {
                        keypad[0] = 1;
                    } break;
                    case SDLK_1:
                    {
                        keypad[1] = 1;
                    } break;
                    case SDLK_2:
                    {
                        keypad[2] = 1;
                    } break;
                    case SDLK_3:
                    {
                        keypad[3] = 1;
                    } break;
                    case SDLK_q:
                    {
                        keypad[4] = 1;
                    } break;
                    case SDLK_w:
                    {
                        keypad[5] = 1;
                    } break;
                    case SDLK_e:
                    {
                        keypad[6] = 1;
                    } break;
                    case SDLK_a:
                    {
                        keypad[7] = 1;
                    } break;
                    case SDLK_s:
                    {
                        keypad[8] = 1;
                    } break;
                    case SDLK_d:
                    {
                        keypad[9] = 1;
                    } break;
                    case SDLK_z:
                    {
                        keypad[0xA] = 1;
                    } break;
                    case SDLK_c:
                    {
                        keypad[0xB] = 1;
                    } break;
                    case SDLK_4:
                    {
                        keypad[0xC] = 1;
                    } break;
                    case SDLK_r:
                    {
                        keypad[0xD] = 1;
                    } break;
                    case SDLK_f:
                    {
                        keypad[0xE] = 1;
                    } break;
                    case SDLK_v:
                    {
                        keypad[0xF] = 1;
                    } break;
                }
            } break;

            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym) {
                    case SDLK_x:
                    {
                        keypad[0] = 0;
                    } break;
                    case SDLK_1:
                    {
                        keypad[1] = 0;
                    } break;
                    case SDLK_2:
                    {
                        keypad[2] = 0;
                    } break;
                    case SDLK_3:
                    {
                        keypad[3] = 0;
                    } break;
                    case SDLK_q:
                    {
                        keypad[4] = 0;
                    } break;
                    case SDLK_w:
                    {
                        keypad[5] = 0;
                    } break;
                    case SDLK_e:
                    {
                        keypad[6] = 0;
                    } break;
                    case SDLK_a:
                    {
                        keypad[7] = 0;
                    } break;
                    case SDLK_s:
                    {
                        keypad[8] = 0;
                    } break;
                    case SDLK_d:
                    {
                        keypad[9] = 0;
                    } break;
                    case SDLK_z:
                    {
                        keypad[0xA] = 0;
                    } break;
                    case SDLK_c:
                    {
                        keypad[0xB] = 0;
                    } break;
                    case SDLK_4:
                    {
                        keypad[0xC] = 0;
                    } break;
                    case SDLK_r:
                    {
                        keypad[0xD] = 0;
                    } break;
                    case SDLK_f:
                    {
                        keypad[0xE] = 0;
                    } break;
                    case SDLK_v:
                    {
                        keypad[0xF] = 0;
                    } break;
                }
            } break;
        }
    }

    return quit;
}


// 64 x 32 display
uint8_t display[CHIP8_WIDTH * CHIP8_HEIGHT] = {0};

constexpr uint8_t fontset[80] = {
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

// Where each sprite is 5 rows tall, 8 columns wide
void drawSprite(int x, int y, const uint8_t* sprite, int height)
{
    // Iterate across character height (5)
    for (int row = 0; row < height; ++row) {
        // sprite is an element within fontset
        // byte is a row element within sprite
        uint8_t byte = sprite[row];

        // Iterate across each bit in the byte
        for (int col = 0; col < 8; ++col) {
            if (byte & (0x80 >> col)) {
                // x, y is the starting point
                // col, row offsets from there
                // % ensures screen wrapping + no calling OOB index values
                int px = (x + col) % CHIP8_WIDTH;
                int py = (y + row) % CHIP8_HEIGHT;
                // XOR bitflip to update screen
                display[py * CHIP8_WIDTH + px] ^= 1;
            }
        }
    }
}


#ifdef MAIN

int main()
{
    Platform platform("test", CHIP8_WIDTH * SCALE, CHIP8_HEIGHT * SCALE);

    // Draw digit '0' at (5, 5)
    drawSprite(5, 5, &fontset[0], 5);

    platform.Update(display);

    bool running = true;
    while (running) {
        // Event handler
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
        }

        SDL_Delay(16);              // 60 fps
    }

    
    return 0;
}

#endif