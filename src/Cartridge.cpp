#include "Cartridge.h"
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>

Cartridge::Cartridge() {};

int Cartridge::loadVector(const std::string& path)
{
    //Makes file pointer based on the path parameter that is passed in
    std::ifstream file(path,std::ios::binary);

    //Makes sure file pointer is valid
    if(!file)
    {
        std::cerr << "Could not open file!\n";
        return -1;
    }

    //Open ROM file and dump contents into ROM vector defined in Cartridge.h
    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),std::back_inserter(rom));
    
    //Get the MBC type for memory stuff
    //Also defined in Cartridge.h
    mbcType = rom[0x0147];
    return 0;

}

void Cartridge::write_8(uint16_t address, uint8_t val)
{
    //Make sure the address is within the ROM vectors range
    //If it write it to the vector
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

/////////////////////////
//Debugging Function   //
/////////////////////////
uint8_t Cartridge::getRom(uint16_t address) {
    
    //Make sure address is within 
    if(address < rom.size())
        return rom[address];
    return 0xFF;
}

//Just gets the mbc type
uint8_t Cartridge::getMBCType() {
    return mbcType;
}