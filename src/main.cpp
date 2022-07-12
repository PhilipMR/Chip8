#include <SDL2/SDL.h>
#include "chip8.h"

int 
main(int argc, char *argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Could not initialize SDL2");
        return -1;
    }

    ch8::GameROM rom("D:/Projects/Chip-8/roms/1dcell.ch8");
    ch8::Chip8Emulator emulator(&rom);
    emulator.Run();

    return 0;
}