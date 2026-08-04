#pragma once
#include <SDL2/SDL.h>
#include <stdio.h>

const int width = 160;
const int height = 144;

class Renderer{
    
    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;
        bool success; //Flag variable

    public:
        bool init();
        void update(uint32_t *buffer);
        void cleanup();
};


