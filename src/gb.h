#pragma once
#include <iostream>
#include "cpu.h"
#include "Bus.h"
#include "registers.h"
#include "Cartridge.h"
#include "ppu.h"
#include "Timer.h"

struct GameBoy
{
    Timer timer;
    Joypad joypad;
    Bus bus;
    Registers regs;
    Cartridge cart;
    Renderer renderer;
    ppu p;
    CPU cpu;
    std::string savePath;

    GameBoy() : bus(&timer, &joypad), p(&bus, &renderer)
    {
        bus.connectCartridge(&cart);
        bus.connectPPU(&p);
        cpu.connectBus(&bus);
        cpu.connectRegisters(&regs);
    }

    bool loadRom(const std::string& path)
    {
        bool ok = cart.loadVector(path) != -1;
        if(ok)
        {
            savePath = path.substr(0, path.find_last_of('.')) + ".sav";
            cart.loadRam(savePath);
        }
        return ok;
    }

    void saveGame()
    {
        cart.saveRam(savePath);
    }

    void init(const std::string& bootRomPath = "../ROMS/bootix_dmg.bin")
    {
        bus.loadBootRom(bootRomPath);
        regs.PC = 0x0000;
        renderer.init();
    }

    void tick()
    {
        uint32_t cycles = cpu.step();
        p.tick(cycles);
        timer.tick(cycles, cpu.cpuStoppedGetter());
        bus.tickDMA(cycles);
    }
    
    void cleanup()
    {
        renderer.cleanup();
    }
};
