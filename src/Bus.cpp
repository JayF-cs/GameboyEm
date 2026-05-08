#include "Bus.h"
#include "Cartridge.h"

void Bus::connectCartridge(Cartridge *c){
    this->cart = c;
}

uint8_t Bus::read_8(uint16_t address) {
    if(address <= 0x7FFF)
    {
        //ROM handled by cartridge
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
        return IORegister[address - 0xFF00];
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
        wram[address -0xC000] = value;
    }
    else if(address <= 0xFDFF)
    {
        //This is echo RAM case
        //Cannot be written to
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
        //Input memory
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