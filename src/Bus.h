//#include <SDL2/SDL.h>
#include <iostream>
#include <cstdint>

class Bus
{
private:
    uint8_t memory[65536];
public:
    
    uint8_t read_8(uint16_t address){

        return memory[address];
    }

    uint16_t read_16(uint16_t address){

        uint8_t low = memory[address];
        uint8_t high = memory[address + 1];

        uint16_t value = (uint16_t)((high << 8) | low);

        return value;
    }

    void write_8( uint16_t address, uint8_t value){

        memory[address] = value;
    }

    void write_16(uint16_t address, uint16_t value){

        memory[address] = (uint8_t)(value & 0xFF);
        memory[address + 1] = (uint8_t)(value >> 8);
    }

};
