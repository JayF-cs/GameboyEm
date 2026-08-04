#include "gb.h"

int main(int argc, char* argv[]){

    if(argc < 2)
    {
        printf("Usage: %s <rom_path.gb> [boot_rom_path.bin]\n", argv[0]);
        return 1;
    }

    GameBoy gb;

    if(!gb.loadRom(argv[1])){ printf("Failed to load ROM: %s\n", argv[1]); return 1;}
    if(argc >= 3)
    {
        gb.init(argv[2]);
    }
    else
    {
        gb.init();
    }


    while(true)
    {

        SDL_Event event;
        while(SDL_PollEvent(&event)) 
        {
            int quit = gb.joypad.handleInput(event);
            if(quit) 
            {
                gb.saveGame();
                gb.cleanup(); 
                exit(0);
            }
        }

        gb.tick();
    }

    return 0;
}
