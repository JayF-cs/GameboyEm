#include "input.h"

void Joypad::updateReg()
{
    uint8_t prevLow = reg & 0x0F;

    if (!(reg & 0x10) && !(reg & 0x20)) {
        reg = (reg & 0xF0) | (dpad & buttons);
    } 
    else if (!(reg & 0x10)) {
        reg = (reg & 0xF0) | dpad;
    } 
    else if (!(reg & 0x20)) {
        reg = (reg & 0xF0) | buttons;
    } 
    else {
        reg = (reg & 0xF0) | 0x0F;
    }

    reg |= 0xC0;

    uint8_t newLow = reg & 0x0F;

    if (IF && (prevLow & ~newLow & 0x0F))
    {
        *IF |= 0x10;
    }
}

int Joypad::handleInput(SDL_Event &event)
{
    if(event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) { return 1; }
 
    if(event.type == SDL_KEYDOWN)
    {
        if(event.key.keysym.sym == SDLK_RIGHT)
        {
            dpad &= ~0x1;
        }

        if(event.key.keysym.sym == SDLK_LEFT)
        {
            dpad &= ~0x2;
        }

        if(event.key.keysym.sym == SDLK_UP)
        {
            dpad &= ~0x4;
        }

        if(event.key.keysym.sym == SDLK_DOWN)
        {
            dpad &= ~0x8;
        }

        if(event.key.keysym.sym == SDLK_z)
        {
            buttons &= ~0x1;
        }

        if(event.key.keysym.sym == SDLK_x)
        {
            buttons &= ~0x2;
        }

        if(event.key.keysym.sym == SDLK_LSHIFT)
        {
            buttons &= ~0x4;
        }

        if(event.key.keysym.sym == SDLK_RETURN)
        {
            buttons &= ~0x8;
        }
    }

    if(event.type == SDL_KEYUP)
    {   
        if(event.key.keysym.sym == SDLK_RIGHT)
        {
            dpad |= 0x1;
        }

        if(event.key.keysym.sym == SDLK_LEFT)
        {
            dpad |= 0x2;
        }

        if(event.key.keysym.sym == SDLK_UP)
        {
            dpad |= 0x4;
        }

        if(event.key.keysym.sym == SDLK_DOWN)
        {
            dpad |= 0x8;
        }

        if(event.key.keysym.sym == SDLK_z)
        {
            buttons |= 0x1;
        }

        if(event.key.keysym.sym == SDLK_x)
        {
            buttons |= 0x2;
        }

        if(event.key.keysym.sym == SDLK_LSHIFT)
        {
            buttons |= 0x4;
        }

        if(event.key.keysym.sym == SDLK_RETURN)
        {
            buttons |= 0x8;
        }
    }

    updateReg();

    return 0;
}