#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include "Cartridge.h"
#include "Timer.h"
#include "input.h"
#include "ppu.h"

class Bus {
private:
    bool dmaActive = false;
    uint16_t dmaSource = 0;
    uint8_t dmaProgress = 0;
    uint8_t dmaCycles = 0;
    uint8_t bootRom[0x100] = {};
    bool bootRomEnabled = true;
    uint8_t vram[0x2000];
    uint8_t highRam[0x007F];
    uint8_t wram[0x2000];
    uint8_t oam[0x00A0];
    uint8_t IORegister[0x0080];
    uint8_t interruptEnable;
    Cartridge *cart = nullptr;
    Timer *t = nullptr;
    Joypad *j = nullptr;
    ppu *p = nullptr;

public:
    Bus(Timer *timer, Joypad *pad) : t(timer), j(pad), vram{}, highRam{}, wram{}, oam{}, IORegister{}, interruptEnable(0) {t->connectIF(&IORegister[0x0F]); j->connectIF(&IORegister[0x0F]);}
    void connectPPU(ppu *p);
    uint8_t read_8(uint16_t address);
    uint16_t read_16(uint16_t address);
    void write_8(uint16_t address, uint8_t value);
    void write_16(uint16_t address, uint16_t value);
    void tickDMA(uint8_t cycles);

    void connectCartridge(Cartridge* c);
    void loadBootRom(const std::string& path);
};