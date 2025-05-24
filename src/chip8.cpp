#include "chip8.h"

Chip8::Chip8() {
}

Chip8::~Chip8() {
}

void Chip8::initialize(){
    //Reset//
    program_Counter = 0x200;
    opcode          = 0;    
    index_Register  = 0;
    stack_pointer   = 0;
    
    //TODO//
    //Clear display, stack registers V0-VF, and memory

    //Load fontset
    for (int i = 0; i < 80; ++i){
        //memory[i] = chip8_fontset[i];
    }
}