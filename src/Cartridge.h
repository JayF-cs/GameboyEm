#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

class Cartridge
{
    private:
        std::vector<uint8_t> rom;
        uint8_t mbcType = 0;
        uint8_t bank0[0x4000];
        uint8_t bank1[0x4000];
        uint8_t external[0x2000];
    public:
        Cartridge();
        int loadVector(const std::string& path);
        void write_8(uint16_t address, uint8_t val);
        uint8_t read_8(uint16_t address);
        uint8_t getRom(uint16_t address);
        uint8_t getMBCType();
};


