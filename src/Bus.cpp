#include "Bus.h"

uint8_t Bus::read_8(uint16_t address) {
    return memory[address];
}

uint16_t Bus::read_16(uint16_t address) {
    uint8_t low  = memory[address];
    uint8_t high = memory[address + 1];
    return (uint16_t)((high << 8) | low);
}

void Bus::write_8(uint16_t address, uint8_t value) {
    memory[address] = value;
}

void Bus::write_16(uint16_t address, uint16_t value) {
    memory[address]     = (uint8_t)(value & 0xFF);
    memory[address + 1] = (uint8_t)(value >> 8);
}