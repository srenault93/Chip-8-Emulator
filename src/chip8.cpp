#include "chip8.h"
#include "opcodes.h"
#include <cstdio>

Chip8::Chip8() {}
Chip8::~Chip8() {}

unsigned short Chip8::get_pc_counter() {
    return program_Counter;
}

bool Chip8::loadRom(const char* file) {
    std::ifstream rom(file, std::ios::binary | std::ios::ate);
    if (!rom.is_open()) {
        std::cerr << "Error: Failed to open ROM: " << file << std::endl;
        return false;
    }

    std::streamsize size = rom.tellg();
    if (size > 4096 - 0x200) {
        std::cerr << "Error: ROM too large (" << size << " bytes)" << std::endl;
        rom.close();
        return false;
    }

    rom.seekg(0, std::ios::beg);
    rom.read(reinterpret_cast<char*>(memory + 0x200), size);
    rom.close();

    printf("[INFO] ROM loaded: %s (%ld bytes)\n", file, (long)size);
    return true;
}

void Chip8::initialize() {
    program_Counter = 0x200;
    opcode          = 0;
    index_Register  = 0;
    stack_pointer   = 0;
    delay_timer     = 0;
    sound_timer     = 0;
    draw_flag       = false;
    state           = RUNNING;

    for (int i = 0; i < 64 * 32; ++i) gfx[i]            = 0;
    for (int i = 0; i < 16;       ++i) CPU_Registers[i]  = 0;
    for (int i = 0; i < 16;       ++i) key[i]            = 0;
    for (int i = 0; i < 16;       ++i) stack[i]          = 0;
    for (int i = 0; i < 4096;     ++i) memory[i]         = 0;

    // Load fontset into 0x050–0x09F
    for (int i = 0; i < 80; ++i) memory[i + 0x50] = chip8_fontset[i];
}

// ─────────────────────────────────────────────
//  emulateCycle — fetch, decode, execute
// ─────────────────────────────────────────────
void Chip8::emulateCycle() {
    // Fetch: combine two bytes into a 16-bit opcode
    opcode = (memory[program_Counter] << 8) | memory[program_Counter + 1];

    // Decode & execute via OpcodeHandler
    OpcodeHandler handler(*this);
    handler.execute(opcode);
}

// ─────────────────────────────────────────────
//  updateTimer — decrement timers at 60 Hz
//  Call this once per frame (not per cycle)
// ─────────────────────────────────────────────
void Chip8::updateTimer() {
    if (delay_timer > 0) --delay_timer;

    if (sound_timer > 0) {
        if (sound_timer == 1)
            printf("[BEEP]\n"); // placeholder until audio is wired
        --sound_timer;
    }
}

// Stubs — to be implemented in later steps
void Chip8::render()      {}
void Chip8::handleInput() {}