#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>

class OpcodeHandler; // Forward declaration

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
    bool draw_flag; // Set true when DXYN executes; cleared after render

    private:

    friend class OpcodeHandler; // Allow full access to private state

    unsigned short opcode;
    
    unsigned char CPU_Registers[16]; // V0-VF; VF is carry/flag register

    unsigned short index_Register;

    unsigned short program_Counter;

    //Memory map//
    //0x000-01FF - Chip 8 interpreter
    //0x050-0x0A0 Used for the 4x5 font set (0-F)
    //0x200-0xFFF - Program ROM and work RAM
    unsigned char memory[4096];

    //Timer registers//
    //When set above 0 they will count down to 0
    unsigned char delay_timer;
    unsigned char sound_timer;

    //Graphics system: 64x32 pixels, each byte is 0 (off) or 1 (on)
    unsigned char gfx[64 * 32];
    
    //Stack: used to remember PC before a jump/subroutine
    unsigned short stack[16];
    unsigned short stack_pointer;
    
    //Hex keypad 0x0-0xF: 1 = pressed, 0 = released
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