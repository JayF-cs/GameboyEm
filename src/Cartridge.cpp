#include "Cartridge.h"
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>

Cartridge::Cartridge() {};

int Cartridge::loadVector(const std::string& path)
{
    std::ifstream file(path,std::ios::binary);

    if(!file)
    {
        std::cerr << "Could not open file!\n";
        return -1;
    }

    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),std::back_inserter(rom));
    mbcType = rom[0x0147];
    return 0;

}

void Cartridge::write_8(uint16_t address, uint8_t val)
{
    if(address < rom.size())
        rom[address] = val;
}

uint8_t Cartridge::read_8(uint16_t address)
{
    if(address < rom.size())
        return rom[address];
    else
        return 0xFF;
}

uint8_t Cartridge::getRom(uint16_t address) {
    if(address < rom.size())
        return rom[address];
    return 0xFF;
}

uint8_t Cartridge::getMBCType() {
    return mbcType;
}