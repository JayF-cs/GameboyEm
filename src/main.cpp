//#include <SDL2/SDL.h>
#include <iostream>
#include "cpu.h"
#include "Bus.h"
#include "registers.h"

int main(){

    Bus bus;
    Bus *b = &bus;
    registers regs;
    registers *r = &regs;

    CPU cpu;
    cpu.connectBus(b);
    cpu.connectRegisters(r);
    return 0;
}