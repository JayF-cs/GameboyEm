#pragma once
#include <cstdint>
#include <vector>
#include <queue>
//#include <SDL2/SDL.h>
#include "Bus.h"

enum class PPU_modes : uint8_t
{
    H_blank, //MODE 0
    V_blank, //MODE 1
    getObjects, //MODE 2
    display //MODE 3
};

struct object
{
    uint8_t x;
    uint8_t y;
    uint8_t tIndex;
    uint8_t flags;
};

struct objPixel
{
    uint8_t colorIndex;
    uint16_t palette;
    uint8_t flags;
};


class ppu
{

    private:
        Bus* b;
        PPU_modes mode;
        std::vector<object> objects;
        std::queue<uint8_t> bgFIFO;
        std::queue<objPixel> obFIFO;
        uint8_t buffer[160];
        uint16_t dots;
        int tileCount;

    public:
        ppu(Bus* bus) : b(bus) {}
        //HBlank - Mode 0
        void mode0();
        //VBlank - Mode 1
        void mode1();
        //Adds objects on current scanline to objects vector
        void mode2();
        //Renders scanlines
        void mode3();
        void initialEnqueues();
        void pixelFetcher();
        //Fetches a pixel from VRAM
        uint8_t getTileFromVRAM(int currentX, int pixelNum);
        uint8_t getObjFromVRAM(object obj, int currentX, int pixelNum);
        //Checks which pixel has priority between a BGP and OBP
        uint8_t priotiryChecker(uint8_t BGP_index, uint8_t OBJ_index, uint8_t flags);
        uint8_t getColor(uint16_t paletteAddr, uint8_t paletteIndex);
        void compareLY();
};