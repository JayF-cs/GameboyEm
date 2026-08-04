#pragma once
#include <iostream>
#include <cstdint>
#include "SDL2/SDL.h"

struct Joypad 
{
    uint8_t reg = 0xFF;
    uint8_t dpad = 0x0F;
    uint8_t buttons = 0x0F;
    uint8_t *IF = nullptr;

    void connectIF(uint8_t *ifMem) {IF = ifMem;}

    void write(uint8_t val) 
    { 
        reg = 0xC0 | (val & 0x30); 
        if(!(reg & 0x10)) reg = (reg & 0xF0) | dpad;
        else if(!(reg & 0x20)) reg = (reg & 0xF0) | buttons;
        else reg = (reg & 0xF0) | 0x0F;
    }
    uint8_t read() { return reg;}
    int handleInput(SDL_Event& e);

    private:
        void updateReg();
};
