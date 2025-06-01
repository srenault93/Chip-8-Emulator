#ifndef OPCODES_H
#define OPCODES_H
#include <cstdint>

class Chip8; // Forward declaration

class OpcodeHandler {
public:
   

    // Constructor, takes reference to Chip8 instance
    OpcodeHandler(Chip8& chip8);

    // Decode and execute a 16-bit opcode
    void execute(uint16_t opcode);

private:
    Chip8& chip8_; // Reference to Chip8 for state access

    // Opcode group handlers
    void code0xxx(uint16_t opcode); // 0NNN, 00E0, 00EE
    void codeNNN(uint16_t opcode); // 1NNN: Jump to NNN
    void code2NNN(uint16_t opcode); // 2NNN: Call subroutine
    void code3XNN(uint16_t opcode); // 3XNN: Skip if VX == NN
    void code4XNN(uint16_t opcode); // 4XNN: Skip if VX != NN
    void code5XY0(uint16_t opcode); // 5XY0: Skip if VX == VY
    void code6XNN(uint16_t opcode); // 6XNN: Set VX to NN
    void code7XNN(uint16_t opcode); // 7XNN: Add NN to VX
    void code8XYN(uint16_t opcode); // 8XY0-8XYE: Arithmetic/logic
    void code9XY0(uint16_t opcode); // 9XY0: Skip if VX != VY
    void codeANNN(uint16_t opcode); // ANNN: Set I to NNN
    void codeBNNN(uint16_t opcode); // BNNN: Jump to NNN + V0
    void codeCXNN(uint16_t opcode); // CXNN: Random number AND NN
    void codeDXYN(uint16_t opcode); // DXYN: Draw sprite
    void codeEXNN(uint16_t opcode); // EX9E, EXA1: Key input
    void codeFXNN(uint16_t opcode); // FX07-FX65: Timer, memory, etc.
};

#endif