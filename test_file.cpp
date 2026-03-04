// TO COMPILE, YOU MUST MANUALLY LINK SDL2 AND GL
// ADD THESE FLAGS DURING COMPILATION
// -lSDL2 -lGL

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <cstdint>
#include <cstring>

constexpr int CHIP8_WIDTH  = 64;
constexpr int CHIP8_HEIGHT = 32;
constexpr int SCALE        = 10;

// 64 x 32 display
// Each pixel is expanded to a 10x10
// Row-major: display[y * 64 + x]
// -- each y advances another 64 pixels, or one row. each x chooses within row
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

// Used to render the frame buffer to OpenGL
// Separate from the display from before, this is what actually gets displayed in the window
void renderDisplay()
{
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
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);   // Init SDL video subsystem

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Renderer",
        SDL_WINDOWPOS_CENTERED, // Center window
        SDL_WINDOWPOS_CENTERED, // Center window
        CHIP8_WIDTH * SCALE,    // Window width
        CHIP8_HEIGHT * SCALE,   // Window height
        SDL_WINDOW_OPENGL       // Enable OpenGL rendering
    );

    // Allow OpenGL to draw in the SDL window
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    // OpenGL 2D setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, CHIP8_WIDTH * SCALE, CHIP8_HEIGHT * SCALE, 0, -1, 1);

    // Reset OpenGL transformations
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw digit '0' at (5, 5)
    drawSprite(5, 5, &fontset[0], 5);

    bool running = true;
    while (running) {
        // Event handler
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
        }

        renderDisplay();            // Draw frame buffer
        SDL_GL_SwapWindow(window);  // Swap buffers
        SDL_Delay(16);              // 60 fps
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
