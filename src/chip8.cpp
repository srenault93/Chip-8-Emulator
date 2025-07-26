#include "chip8.h"

Chip8::Chip8() {
}

Chip8::~Chip8() {
}


unsigned short Chip8::get_pc_counter(){
    return Chip8::program_Counter;
}

bool Chip8::loadRom(const char* file) {
    
    std::ifstream rom(file, std::ios::binary | std::ios::ate);
    if (!rom.is_open()){
        std::cerr << "Error: Failure to open ROM" << file << std::endl;
        return false;
    } 

    
    std::streamsize size = rom.tellg();

    if (size > 4096 - 0x200){
        std::cerr << "Error: File too large";
        rom.close();
        return false;
    }

    //set pointer back to start of file
    rom.seekg(0, std::ios::beg);
    
    
    rom.read(reinterpret_cast<char*>(memory + 0x200), size);

    
    rom.close();
    return true;

}


void Chip8::initialize(){

    program_Counter = 0x200;
    opcode          = 0;    
    index_Register  = 0;
    stack_pointer   = 0;
    delay_timer     = 0;
    sound_timer     = 0;


    for (int i = 0; i < 64 * 32; ++i) {gfx[i] = 0;}
    
    for (int i = 0; i < 4096; ++i)    {CPU_Registers[i] = 0; key[i] = 0;}

    for (int i = 0; i < 4096; ++i)    {memory[i] = 0;}

    for (int i = 0; i < 80; ++i)      {memory[i + 0x50] = chip8_fontset[i];}
}

