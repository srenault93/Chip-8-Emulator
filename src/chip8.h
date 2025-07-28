#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>

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
    unsigned short get_pc_counter();

    enum emu_state {
        QUIT,
        RUNNING
    };

    emu_state state;

    private:

    unsigned short opcode;
    
    unsigned char CPU_Registers[16];//V0, V1,...,VE 16th reg is the carry flag

    unsigned short index_Register;

    unsigned short program_Counter;

    //Memory map//
    //0x000-01FF - Chip 8 interpreter
    //0x050-0x0A0 Used for the 4x5 font set (0-f)
    //0x200-oxFFF - Program ROM and work RAM
    unsigned char memory[4096];

    //Timer registers//
    //When set above 0 they will count down to 0

    unsigned char delay_timer;

    unsigned char sound_timer;

    //Graphics system 2048 pixels
    unsigned char gfx[64 * 32];
    
    //Stack//
    //Used to remember current locaton before a jump is performed
    //MUST BE CALLED before any jump or subroutine!!

    unsigned short stack[16];
    unsigned short stack_pointer;
    
    //Hex based keypad (0x0-0-F) used to story current state of key
    unsigned char key[16];


    unsigned char chip8_fontset[80] =
    { 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

};


#endif