#include "opcodes.h"
#include "chip8.h"
#include <cstdlib>   // rand()
#include <cstdio>    // fprintf, stderr

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
OpcodeHandler::OpcodeHandler(Chip8& chip8) : chip8_(chip8) {}

// ─────────────────────────────────────────────
//  Top-level decode: route by upper nibble
// ─────────────────────────────────────────────
void OpcodeHandler::execute(uint16_t opcode) {
    switch (opcode & 0xF000) {
        case 0x0000: code0xxx(opcode); break;
        case 0x1000: codeNNN (opcode); break;
        case 0x2000: code2NNN(opcode); break;
        case 0x3000: code3XNN(opcode); break;
        case 0x4000: code4XNN(opcode); break;
        case 0x5000: code5XY0(opcode); break;
        case 0x6000: code6XNN(opcode); break;
        case 0x7000: code7XNN(opcode); break;
        case 0x8000: code8XYN(opcode); break;
        case 0x9000: code9XY0(opcode); break;
        case 0xA000: codeANNN(opcode); break;
        case 0xB000: codeBNNN(opcode); break;
        case 0xC000: codeCXNN(opcode); break;
        case 0xD000: codeDXYN(opcode); break;
        case 0xE000: codeEXNN(opcode); break;
        case 0xF000: codeFXNN(opcode); break;
        default:
            fprintf(stderr, "[WARN] Unknown opcode: 0x%04X\n", opcode);
            break;
    }
}

// ─────────────────────────────────────────────
//  0x0000 group
// ─────────────────────────────────────────────
void OpcodeHandler::code0xxx(uint16_t opcode) {
    switch (opcode & 0x00FF) {
        case 0xE0:
            // 00E0 — Clear the display
            for (int i = 0; i < 64 * 32; ++i)
                chip8_.gfx[i] = 0;
            chip8_.draw_flag = true;
            chip8_.program_Counter += 2;
            break;

        case 0xEE:
            // 00EE — Return from subroutine
            --chip8_.stack_pointer;
            chip8_.program_Counter = chip8_.stack[chip8_.stack_pointer];
            chip8_.program_Counter += 2;
            break;

        default:
            // 0NNN — Call machine code routine (ignored in most emulators)
            fprintf(stderr, "[INFO] 0NNN opcode 0x%04X ignored\n", opcode);
            chip8_.program_Counter += 2;
            break;
    }
}

// ─────────────────────────────────────────────
//  1NNN — Jump to address NNN
// ─────────────────────────────────────────────
void OpcodeHandler::codeNNN(uint16_t opcode) {
    chip8_.program_Counter = opcode & 0x0FFF;
}

// ─────────────────────────────────────────────
//  2NNN — Call subroutine at NNN
// ─────────────────────────────────────────────
void OpcodeHandler::code2NNN(uint16_t opcode) {
    chip8_.stack[chip8_.stack_pointer] = chip8_.program_Counter;
    ++chip8_.stack_pointer;
    chip8_.program_Counter = opcode & 0x0FFF;
}

// ─────────────────────────────────────────────
//  3XNN — Skip next if VX == NN
// ─────────────────────────────────────────────
void OpcodeHandler::code3XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;
    chip8_.program_Counter += (chip8_.CPU_Registers[x] == nn) ? 4 : 2;
}

// ─────────────────────────────────────────────
//  4XNN — Skip next if VX != NN
// ─────────────────────────────────────────────
void OpcodeHandler::code4XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;
    chip8_.program_Counter += (chip8_.CPU_Registers[x] != nn) ? 4 : 2;
}

// ─────────────────────────────────────────────
//  5XY0 — Skip next if VX == VY
// ─────────────────────────────────────────────
void OpcodeHandler::code5XY0(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    chip8_.program_Counter += (chip8_.CPU_Registers[x] == chip8_.CPU_Registers[y]) ? 4 : 2;
}

// ─────────────────────────────────────────────
//  6XNN — Set VX = NN
// ─────────────────────────────────────────────
void OpcodeHandler::code6XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;
    chip8_.CPU_Registers[x] = nn;
    chip8_.program_Counter += 2;
}

// ─────────────────────────────────────────────
//  7XNN — VX += NN  (no carry flag)
// ─────────────────────────────────────────────
void OpcodeHandler::code7XNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;
    chip8_.CPU_Registers[x] += nn;   // wraps naturally as uint8_t
    chip8_.program_Counter += 2;
}

// ─────────────────────────────────────────────
//  8XYN — ALU operations
// ─────────────────────────────────────────────
void OpcodeHandler::code8XYN(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n =  opcode & 0x000F;

    switch (n) {
        case 0x0: // 8XY0 — VX = VY
            chip8_.CPU_Registers[x] = chip8_.CPU_Registers[y];
            break;

        case 0x1: // 8XY1 — VX |= VY
            chip8_.CPU_Registers[x] |= chip8_.CPU_Registers[y];
            chip8_.CPU_Registers[0xF] = 0; // VIP resets VF
            break;

        case 0x2: // 8XY2 — VX &= VY
            chip8_.CPU_Registers[x] &= chip8_.CPU_Registers[y];
            chip8_.CPU_Registers[0xF] = 0;
            break;

        case 0x3: // 8XY3 — VX ^= VY
            chip8_.CPU_Registers[x] ^= chip8_.CPU_Registers[y];
            chip8_.CPU_Registers[0xF] = 0;
            break;

        case 0x4: { // 8XY4 — VX += VY, VF = carry
            uint16_t sum = chip8_.CPU_Registers[x] + chip8_.CPU_Registers[y];
            chip8_.CPU_Registers[x]    = sum & 0xFF;
            chip8_.CPU_Registers[0xF]  = (sum > 0xFF) ? 1 : 0;
            break;
        }

        case 0x5: { // 8XY5 — VX -= VY, VF = NOT borrow
            uint8_t vx = chip8_.CPU_Registers[x];
            uint8_t vy = chip8_.CPU_Registers[y];
            chip8_.CPU_Registers[x]   = vx - vy;
            chip8_.CPU_Registers[0xF] = (vx >= vy) ? 1 : 0;
            break;
        }

        case 0x6: { // 8XY6 — VX >>= 1, VF = shifted-out bit
            // Most-compatible: operate on VX directly (ignore VY)
            uint8_t lsb = chip8_.CPU_Registers[x] & 0x1;
            chip8_.CPU_Registers[x] >>= 1;
            chip8_.CPU_Registers[0xF] = lsb;
            break;
        }

        case 0x7: { // 8XY7 — VX = VY - VX, VF = NOT borrow
            uint8_t vx = chip8_.CPU_Registers[x];
            uint8_t vy = chip8_.CPU_Registers[y];
            chip8_.CPU_Registers[x]   = vy - vx;
            chip8_.CPU_Registers[0xF] = (vy >= vx) ? 1 : 0;
            break;
        }

        case 0xE: { // 8XYE — VX <<= 1, VF = shifted-out bit
            // Most-compatible: operate on VX directly (ignore VY)
            uint8_t msb = (chip8_.CPU_Registers[x] >> 7) & 0x1;
            chip8_.CPU_Registers[x] <<= 1;
            chip8_.CPU_Registers[0xF] = msb;
            break;
        }

        default:
            fprintf(stderr, "[WARN] Unknown 8XYN opcode: 0x%04X\n", opcode);
            break;
    }
    chip8_.program_Counter += 2;
}

// ─────────────────────────────────────────────
//  9XY0 — Skip next if VX != VY
// ─────────────────────────────────────────────
void OpcodeHandler::code9XY0(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    chip8_.program_Counter += (chip8_.CPU_Registers[x] != chip8_.CPU_Registers[y]) ? 4 : 2;
}

// ─────────────────────────────────────────────
//  ANNN — I = NNN
// ─────────────────────────────────────────────
void OpcodeHandler::codeANNN(uint16_t opcode) {
    chip8_.index_Register = opcode & 0x0FFF;
    chip8_.program_Counter += 2;
}

// ─────────────────────────────────────────────
//  BNNN — Jump to NNN + V0
// ─────────────────────────────────────────────
void OpcodeHandler::codeBNNN(uint16_t opcode) {
    chip8_.program_Counter = (opcode & 0x0FFF) + chip8_.CPU_Registers[0];
}

// ─────────────────────────────────────────────
//  CXNN — VX = rand() & NN
// ─────────────────────────────────────────────
void OpcodeHandler::codeCXNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;
    chip8_.CPU_Registers[x] = (rand() % 256) & nn;
    chip8_.program_Counter += 2;
}

// ─────────────────────────────────────────────
//  DXYN — Draw N-row sprite at (VX, VY)
//  Sprite data starts at memory[I].
//  Pixels are XOR'd onto gfx[]. VF = 1 if any
//  pixel was erased (collision), else 0.
// ─────────────────────────────────────────────
void OpcodeHandler::codeDXYN(uint16_t opcode) {
    uint8_t x      = (opcode & 0x0F00) >> 8;
    uint8_t y      = (opcode & 0x00F0) >> 4;
    uint8_t height =  opcode & 0x000F;

    uint8_t vx = chip8_.CPU_Registers[x] % 64; // wrap to screen width
    uint8_t vy = chip8_.CPU_Registers[y] % 32; // wrap to screen height

    chip8_.CPU_Registers[0xF] = 0; // reset collision flag

    for (int row = 0; row < height; ++row) {
        uint8_t sprite_byte = chip8_.memory[chip8_.index_Register + row];

        for (int col = 0; col < 8; ++col) {
            // Check each bit of the sprite byte, MSB first
            if ((sprite_byte & (0x80 >> col)) != 0) {
                int px = (vx + col) % 64; // wrap horizontally
                int py = (vy + row) % 32; // wrap vertically
                int idx = py * 64 + px;

                if (chip8_.gfx[idx] == 1)
                    chip8_.CPU_Registers[0xF] = 1; // collision

                chip8_.gfx[idx] ^= 1;
            }
        }
    }

    chip8_.draw_flag = true;
    chip8_.program_Counter += 2;
}

// ─────────────────────────────────────────────
//  EXNN — Key-based skip instructions
// ─────────────────────────────────────────────
void OpcodeHandler::codeEXNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;
    uint8_t key = chip8_.CPU_Registers[x] & 0xF; // clamp to valid key index

    switch (nn) {
        case 0x9E: // EX9E — Skip if key[VX] is pressed
            chip8_.program_Counter += chip8_.key[key] ? 4 : 2;
            break;

        case 0xA1: // EXA1 — Skip if key[VX] is NOT pressed
            chip8_.program_Counter += chip8_.key[key] ? 2 : 4;
            break;

        default:
            fprintf(stderr, "[WARN] Unknown EXNN opcode: 0x%04X\n", opcode);
            chip8_.program_Counter += 2;
            break;
    }
}

// ─────────────────────────────────────────────
//  FXNN — Misc: timers, I, BCD, memory
// ─────────────────────────────────────────────
void OpcodeHandler::codeFXNN(uint16_t opcode) {
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t nn =  opcode & 0x00FF;

    switch (nn) {
        case 0x07: // FX07 — VX = delay_timer
            chip8_.CPU_Registers[x] = chip8_.delay_timer;
            chip8_.program_Counter += 2;
            break;

        case 0x0A: { // FX0A — Wait for key press, store in VX
            // Stall by NOT advancing PC until a key is detected
            bool key_pressed = false;
            for (int i = 0; i < 16; ++i) {
                if (chip8_.key[i]) {
                    chip8_.CPU_Registers[x] = i;
                    key_pressed = true;
                    break;
                }
            }
            if (key_pressed)
                chip8_.program_Counter += 2;
            // else: PC stays, instruction re-executes next cycle
            break;
        }

        case 0x15: // FX15 — delay_timer = VX
            chip8_.delay_timer = chip8_.CPU_Registers[x];
            chip8_.program_Counter += 2;
            break;

        case 0x18: // FX18 — sound_timer = VX
            chip8_.sound_timer = chip8_.CPU_Registers[x];
            chip8_.program_Counter += 2;
            break;

        case 0x1E: { // FX1E — I += VX  (VF not set; most compatible)
            chip8_.index_Register += chip8_.CPU_Registers[x];
            chip8_.program_Counter += 2;
            break;
        }

        case 0x29: // FX29 — I = address of font sprite for digit VX
            // Fontset starts at 0x50, each character is 5 bytes
            chip8_.index_Register = 0x50 + (chip8_.CPU_Registers[x] & 0xF) * 5;
            chip8_.program_Counter += 2;
            break;

        case 0x33: // FX33 — Store BCD of VX at I, I+1, I+2
            chip8_.memory[chip8_.index_Register]     =  chip8_.CPU_Registers[x] / 100;
            chip8_.memory[chip8_.index_Register + 1] = (chip8_.CPU_Registers[x] / 10) % 10;
            chip8_.memory[chip8_.index_Register + 2] =  chip8_.CPU_Registers[x] % 10;
            chip8_.program_Counter += 2;
            break;

        case 0x55: // FX55 — Store V0..VX in memory starting at I
            // Most-compatible: I is NOT modified after the operation
            for (int i = 0; i <= x; ++i)
                chip8_.memory[chip8_.index_Register + i] = chip8_.CPU_Registers[i];
            chip8_.program_Counter += 2;
            break;

        case 0x65: // FX65 — Read V0..VX from memory starting at I
            // Most-compatible: I is NOT modified after the operation
            for (int i = 0; i <= x; ++i)
                chip8_.CPU_Registers[i] = chip8_.memory[chip8_.index_Register + i];
            chip8_.program_Counter += 2;
            break;

        default:
            fprintf(stderr, "[WARN] Unknown FXNN opcode: 0x%04X\n", opcode);
            chip8_.program_Counter += 2;
            break;
    }
}