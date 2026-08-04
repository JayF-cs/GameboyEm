#pragma once
#include <cstdint>
#include <vector>
#include <queue>

#include "render.h"

class Bus;

enum class PPU_modes : uint8_t
{
    MODE_0, //H Blank
    MODE_1, //V Blank
    MODE_2, //Get Objects
    MODE_3 //Write to buffer
};

enum class GBColors : uint8_t
{
    WHITE,
    LIGHT_GRAY,
    DARK_GRAY,
    BLACK
};

struct object
{
    uint8_t x;
    int16_t y;
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
        Renderer *r;
        PPU_modes mode = PPU_modes::MODE_1;
        std::vector<object> objects;
        std::queue<uint8_t> bgFIFO;
        std::queue<objPixel> obFIFO;
        uint8_t buffer[160];
        uint32_t frame[144][160];
        int16_t dots = 384;
        bool prevPowerState = false;
        bool startUp = false;
        int window_ly = 0;
        bool window_trigger = false;
        int tileCount = 0;
        uint8_t lcdc = 0x00; 
        uint8_t stat = 0x85;
        uint8_t scy = 0x00;
        uint8_t scx = 0x00;
        uint8_t ly = 0x99;
        uint8_t lyc = 0x00;
        uint8_t wy = 0x00;
        uint8_t wx = 0x00;
        uint8_t bgp  = 0xFC;
        uint8_t obp0 = 0xFF;
        uint8_t obp1 = 0xFF;

    public:
        ppu(Bus* bus, Renderer* renderer) : b(bus), r(renderer) {}
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
        //Checks which pixel has priority between a BGP and OBP returns bool for BGP and false for OBP
        bool priotiryChecker(uint8_t BGP_index, uint8_t OBJ_index, uint8_t flags);
        uint8_t getColor(uint16_t paletteAddr, uint8_t paletteIndex);
        void compareLY();
        void tick(int cycles);
        void setModeSTAT();
        void frameColor(GBColors color, uint8_t row, uint8_t col);
        void lineToRender();
        auto getDot(){return dots;}
        void setDots();
        void setMode(PPU_modes NewMode);
        uint8_t readReg(uint16_t addr);
        void writeReg(uint16_t addr, uint8_t val);
};