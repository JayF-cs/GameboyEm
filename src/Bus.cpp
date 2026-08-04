#include "Bus.h"

void Bus::connectCartridge(Cartridge *c){this->cart = c;}
void Bus::connectPPU(ppu *p){this->p = p;}

uint8_t Bus::read_8(uint16_t address) {
    
    // if(address == 0xFF04) {
    //     fprintf(stderr, "Read DIV: val=0x%02X PC=0x%04X OP=0x%02X step=%llu\n",
    //     t->getDiv(), g_debugPC, g_debugOpcode, g_debugStep);
    // }
    
    if(address <= 0x7FFF)
    {
        //ROM handled by cartridge
        if(bootRomEnabled && address <= 0x00FF) return bootRom[address];
        return cart->read_8(address);
    }
    else if(address <= 0x9FFF)
    {
        //Video RAM
        return vram[address-0x8000];
    }
    else if(address <= 0xBFFF)
    {
        //External RAM
        return cart->read_8(address);
    }
    else if(address <= 0xDFFF)
    {
        //WRAM
        return wram[address - 0xC000];
    }
    else if(address <= 0xFDFF)
    {
        //This is echo RAM case
        //Just mirrors first 4KiB of WRAM
        return wram[address - 0xE000];
    }
    else if(address <= 0xFE9F)
    {
        //Object Attribute Memory
        return oam[address - 0xFE00];
    }
    else if(address <= 0xFEFF)
    {
        //Inaccessbile Memory
        return 0xFF;
    }
    else if(address <= 0xFF7F)
    {
        //Input Memory
        if(address == 0xFF00) return j->read();
        if(address == 0xFF04) return t->getDiv();
        if(address == 0xFF05) return t->getCounter();
        if(address == 0xFF06) return t->getModulo();
        if(address == 0xFF07) return t->getControl();
        if(address == 0xFF0F) return IORegister[address - 0xFF00] | 0xE0;
        if((address >= 0xFF40 && address <= 0xFF4B) && address != 0xFF64) return p->readReg(address);

        if(address >= 0xFF10 && address <= 0xFF3F) return IORegister[address - 0xFF00];

        //All the rest of the registers are CGB only so fall through to unmapped on DMG
        return 0xFF;
    }
    else if (address <= 0xFFFE)
    {
        //High RAM
        return highRam[address - 0xFF80];
    }
    else
    {
        //Interupt handling
        return interruptEnable;
    }
}

uint16_t Bus::read_16(uint16_t address) {
    
    //Make temp vars for lower 8 bits an higher 8 bits
    uint8_t low = read_8(address);
    uint8_t high = read_8(address + 1);
    return (uint16_t)((high << 8) | low);
}

void Bus::write_8(uint16_t address, uint8_t value) {

    if(address <= 0x7FFF)
    {
        //ROM handled by cartridge
        cart->write_8(address, value);
    }
    else if(address <= 0x9FFF)
    {
        //Video RAM
        vram[address-0x8000] = value;
    }
    else if(address <= 0xBFFF)
    {
        //External RAM
        cart->write_8(address,value);
    }
    else if(address <= 0xDFFF)
    {
        //WRAM
        wram[address - 0xC000] = value;
    }
    else if(address <= 0xFDFF)
    {
        //This is echo RAM case
        //Cannot be written to
        wram[address - 0xE000] = value;
        return;
    }
    else if(address <= 0xFE9F)
    {
        //Object Attribute Memory
        oam[address - 0xFE00] = value;
    }
    else if(address <= 0xFEFF)
    {
        //Inaccessible memory
        return;
    }
    else if(address <= 0xFF7F)
    {
        if(address == 0xFF00)
        {
            j->write(value);
            return;
        }
        //Input memory
        if(address == 0xFF04)
        {
            t->setDiv(0);
            return;
        }
        
        if(address == 0xFF05)
        {
            t->setCounter(value);
            return;
        }

        if(address == 0xFF06)
        {
            t->setModulo(value);
            return;
        }

        if(address == 0xFF07)
        {
            t->setControl(value);
            return;
        }

        if(address == 0xFF0F)
        {
            IORegister[address - 0xFF00] = value | 0xE0;
            return;
        }

        if((address >= 0xFF40 && address <= 0xFF4B) && address != 0xFF46)
        {
            p->writeReg(address, value); 
            return;
        }
        
        if(address == 0xFF46)
        {
            dmaSource = (uint16_t)value << 8;
            dmaProgress = 0;
            dmaActive = true;
            IORegister[address - 0xFF00] = value;
            return;
        }

        if(address == 0xFF50)
        {
            bootRomEnabled = false;
            return;
        }

        IORegister[address - 0xFF00] = value;

    }
    else if (address <= 0xFFFE)
    {
        //High RAM
        highRam[address - 0xFF80] = value;
    }
    else
    {
        interruptEnable = value;
    }
}

void Bus::write_16(uint16_t address, uint16_t value) {
    write_8(address, (uint8_t)(value & 0xFF));
    write_8(address + 1, (uint8_t)(value >> 8));
}

void Bus::tickDMA(uint8_t cycles)
{
    if(!dmaActive) return;
    dmaCycles += cycles;

    while(dmaCycles >= 4 && dmaActive)
    {
        dmaCycles -= 4;
        oam[dmaProgress] = read_8(dmaSource + dmaProgress);
        dmaProgress++;

        if(dmaProgress >= 0xA0)
        {
            dmaActive = false;
            dmaCycles = 0;
        }
    }

}

void Bus::loadBootRom(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if(!file) { fprintf(stderr, "Failed to open boot ROM: %s\n", path.c_str()); return; }
    file.read(reinterpret_cast<char*>(bootRom), 0x100);
}