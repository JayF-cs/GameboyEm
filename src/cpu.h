#pragma once
#include <iostream>
#include <cstdint>
#include <conio.h>

class Bus;

struct Registers;

class CPU {
    public:
        CPU();
        void connectBus(Bus* b);
        void connectRegisters(Registers* reg);
        uint8_t step();
        bool imeGetter();
        bool cpuStoppedGetter();

    private:
        Bus* b = nullptr;
        Registers* r = nullptr;
        uint64_t tCycles = 0;

        const uint8_t FLAG_Z = 1 << 7;
        const uint8_t FLAG_N = 1 << 6;
        const uint8_t FLAG_H = 1 << 5;
        const uint8_t FLAG_C = 1 << 4;

        bool cpu_stopped = false;
        bool isHalted = false;
        bool ime = false;
        int ime_delay;
        bool haltBug = false;
        bool start = false;
        int steps = 0;
        

        // Helper declarations
        uint16_t fetch_16();
        uint8_t fetch_8();
        void setFlag(uint8_t flag, bool val);
        void setZ(bool val);
        void setN(bool val);
        void setH(bool val);
        void setC(bool val);
        uint8_t inc8(uint8_t val);
        uint8_t dec8(uint8_t val);
        void addHL(uint16_t reg);
        bool checkInteruption();
        void add(uint8_t val);
        void sub(uint8_t val);
        void adc(uint8_t val);
        void sbc(uint8_t val);
        void and_a(uint8_t val);
        void xor_a(uint8_t val);
        void or_a(uint8_t val);
        void cmp(uint8_t val);
        uint16_t pop16();
        void push(uint16_t val);
        void interupts();
        int executeCBOpcode(uint8_t extendedInstruction);
        uint8_t swap(uint8_t val);
        uint8_t rlc(uint8_t val);
        uint8_t rl(uint8_t val);
        uint8_t rrc(uint8_t val);
        uint8_t rr(uint8_t val);
        uint8_t sla(uint8_t val);
        uint8_t srl(uint8_t val);
        uint8_t sra(uint8_t val);
        void bit(uint8_t val, uint8_t check);
        uint8_t res(uint8_t val, uint8_t check);
        uint8_t set(uint8_t val, uint8_t check);

};

