#include "render.h"

bool Renderer::init()
{
    success = true;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        window = SDL_CreateWindow("GB Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * 3, height * 3, SDL_WINDOW_SHOWN);

        if(window == NULL)
        {
            printf("Window could not be created! SDL Error : %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if(renderer == NULL)
            {
                printf("Renderer could not be created! SDL ERROR : %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                SDL_RenderSetLogicalSize(renderer, width, height);
                //Set draw color to black
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);

                if(texture == NULL)
                {
                    printf("Texture could not be created! SDL Error : %s\n", SDL_GetError());
                    success = false;
                }
            }
        }
    }

    return success;
}

void Renderer::update(uint32_t *buffer)
{
    SDL_UpdateTexture(texture, NULL, buffer, width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void Renderer::cleanup()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}