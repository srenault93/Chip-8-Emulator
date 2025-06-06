#include "SDL2/SDL.h"
#include "chip8.h"

Chip8 chip8;

void init_SDL(){
    if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0));
}

void cleanup(){}


int main(int argc, char **argv){

    
    

    
    
    //setUpGraphics();
    //setUpInput();

    //Initialize the chip8 and load rom
    //chip8.initialize();
    //chip8.loadRom("example");

    //Emulation loop
    //for(;;){

        //chip8.emulateCycle();

        //if(chip8.drawFlag){
            //drawGraphics();
        //}

        //Chip8.setKeys();
    //}

}
