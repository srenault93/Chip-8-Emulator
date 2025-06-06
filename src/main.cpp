#include "SDL2/SDL.h"
#include "chip8.h"
typedef struct{
    Chip8 chip8;
    SDL_Window *window;
} sdl_T;



bool init_SDL(){
    if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)){
        SDL_Log("Error initializing SDL", SDL_GetError());
        return false;
    }

    return true;
}

void cleanup(){
    SDL_DestroyWindow(window);
    SDL_Quit();
}


int main(int argc, char **argv){

    if (!init_SDL()) exit(EXIT_FAILURE);
    

    
    
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


    cleanup();

}
