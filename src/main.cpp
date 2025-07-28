#include "SDL2/SDL.h"
#include "chip8.h"


typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;




bool init_SDL(sdl_t *sdl){
    
     
    
 
    if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)){
        SDL_Log(SDL_GetError());
        return false;
    }

    sdl -> window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                    500, 500, 0);
    
    
    if (!sdl -> window) {SDL_Log(SDL_GetError()); return false;}    
    

    sdl -> renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl -> renderer) {SDL_Log(SDL_GetError()); return false;}
          

    return true;
}

void cleanup(const sdl_t sdl){
    SDL_DestroyRenderer(sdl.renderer);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit();
}

void clear_display(const sdl_t sdl) {
     SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 0);
     SDL_RenderClear(sdl.renderer);

}

void update_display(const sdl_t sdl){
    SDL_RenderPresent(sdl.renderer);
}

void inputs(Chip8 &chip8){
    SDL_Event event;

    while (SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                chip8.state = Chip8::QUIT;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym){
                    case SDLK_ESCAPE:
                    chip8.state = Chip8::QUIT;
                    chip8.state = Chip8::QUIT;
                }
        }
    }

}


int main(int argc, char **argv){
    Chip8 chip8;
    chip8.initialize();

    sdl_t sdl = {0};
    
    if (!init_SDL(&sdl)) exit(EXIT_FAILURE);


    while (chip8.state != Chip8::QUIT){
        
        inputs(chip8);

        SDL_Delay(16);

        update_display(sdl);
    
    
    }


    cleanup(sdl);

}
