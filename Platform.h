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

        void PlaySound();
        void StopSound();

    private:
        SDL_AudioDeviceID audioDevice; // Handle for the audio output
        SDL_Window* window{};
        SDL_GLContext glContext{};
};

#endif