#include "chip8.h"

Chip8::Chip8() {
}

Chip8::~Chip8() {
}

bool Chip8::loadRom(const char* file) {
    
    //open ROM in binary mode
    std::ifstream rom(file, std::ios::binary | std::ios::ate);
    if (!rom.is_open()){
        std::cerr << "Error: Failure to open ROM" << file << std::endl;
        return false;
    } 

    //Get file size
    std::streamsize size = rom.tellg();
    //checks if size is larger than Max size
    if (size > 4096 - 0x200){
        std::cerr << "Error: File too large";
        rom.close();
        return false;
    }

    //set pointer back to start of file
    rom.seekg(0, std::ios::beg);
    
    //put rom into memory
    rom.read(reinterpret_cast<char*>(memory + 0x200), size);

    
    
    rom.close();
    return true;

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

