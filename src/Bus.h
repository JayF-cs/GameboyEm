#pragma once
#include <cstdint>
#include "Cartridge.h"

class Bus {
private:
    uint8_t vram[0x2000];
    uint8_t highRam[0x007F];
    uint8_t wram[0x2000];
    uint8_t oam[0x00A0];
    uint8_t IORegister[0x0080];
    uint8_t interruptEnable;
    Cartridge *cart = nullptr;
public:
    uint8_t read_8(uint16_t address);
    uint16_t read_16(uint16_t address);
    void write_8(uint16_t address, uint8_t value);
    void write_16(uint16_t address, uint16_t value);
    void connectCartridge(Cartridge* c);
};