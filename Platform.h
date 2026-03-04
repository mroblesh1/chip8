#ifndef PLATFORM_H
#define PLATFORM_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <GL/gl.h>
#include <cstring>

class Platform {
    public:
        Platform(char const* title, int windowWidth, int windowHeight);
        ~Platform();
        void Update(void const* buffer);
        bool ProcessInput(uint8_t* keys);

    private:
        SDL_Window* window{};
        SDL_GLContext glContext{};
};

#endif