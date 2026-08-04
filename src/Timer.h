#pragma once
#include <cstdint>

class Timer {
    
    private:
        uint32_t internal_counter = 0;
        uint8_t tCounter = 0;       
        uint8_t tModulo= 0;
        uint8_t tControl = 0xF8;
        uint16_t getTimer();

        uint8_t *IF = nullptr;
        bool prevBit = false;

    public:
        void tick(uint8_t cycles, bool cpu_stopped);

        void setDiv(uint8_t div);
        void setCounter(uint8_t val);
        void setModulo(uint8_t val);
        void setControl(uint8_t val);

        uint8_t getDiv();
        uint8_t getCounter();
        uint8_t getModulo();
        uint8_t getControl();
        
        void setBootDiv(uint16_t val);

        void connectIF(uint8_t *ifMem);
    
};