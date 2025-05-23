#ifndef CHIP8_HPP
#define CHIP8_HPP
#include <cstdint>
#include <SDL2/SDL.h>

class Chip8 {
    public:

    Chip8();
    ~Chip8();

    bool loadRom(const char* file); // Load ROM into memory
    void emulateCycle (); //fde one instruction
    void render       (); //Draw display to window
    void handleInput  (); // Process key input
    void updateTimer  (); 
    void initialize   ();


    private:

    unsigned char memory[4096];
    

};


#endif