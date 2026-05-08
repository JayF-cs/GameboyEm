//#include <SDL2/SDL.h>
#include <iostream>
#include "cpu.h"
#include "Bus.h"
#include "registers.h"
#include "Cartridge.h"

int main(){

    Bus bus;
    Bus *b = &bus;
    registers regs;
    registers *r = &regs;
    Cartridge cart;
    Cartridge *c = &cart;

    cart.loadVector("./ROMS/Tetris.gb");

    ////////////////////////////////////////////////////////////
    //  Test Loading a ROM                                    //
    ////////////////////////////////////////////////////////////

    // Print cartridge title (stored at 0x134-0x143 in ROM header)
    printf("Title: ");
    for(int i = 0x134; i <= 0x143; i++)
        printf("%c", cart.getRom(i));
    printf("\n");

    // Print MBC type
    printf("MBC Type: 0x%02X\n", cart.getMBCType());

    CPU cpu;
    b->connectCartridge(c);
    cpu.connectBus(b);
    cpu.connectRegisters(r);
    return 0;
}