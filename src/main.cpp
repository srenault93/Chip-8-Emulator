#include <SDL2/SDL.h>
#include "chip8.hpp"

Chip8 chip8;

int main(int argc, char **argv){

    //setUpGraphics();
    //setUpInput();

    //Initialize the chip8 and load rom
    chip8.initialize();
    chip8.loadRom("example");

    //Emulation loop
    for(;;){

        chip8.emulateCycle();

        if(chip8.drawFlag){
            //drawGraphics();
        }

        Chip8.setKeys();
    }

}
