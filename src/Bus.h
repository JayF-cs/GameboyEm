#pragma once
#include <cstdint>

class Bus {
private:
    uint8_t memory[65536]{};
public:
    uint8_t  read_8(uint16_t address);
    uint16_t read_16(uint16_t address);
    void     write_8(uint16_t address, uint8_t value);
    void     write_16(uint16_t address, uint16_t value);
};