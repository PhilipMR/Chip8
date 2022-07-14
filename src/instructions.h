#pragma once 

#include <stdint.h>
#include <unordered_map>
#include <cassert>
#include <random>

#include "memory.h"
#include "registers.h"
#include "display.h"

namespace ch8
{
    static uint8_t
    GenRandomNumber()
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(0,255);
        return dist6(rng);
    }



    void
    ProcessOpCode(uint16_t      opcode,
                  DisplayUnit*  display, 
                  RegisterUnit* registers, 
                  MemoryUnit*   memory)
    {
        const uint8_t msb = (opcode & 0xFF00) >> 8;
        const uint8_t lsb = (opcode & 0x00FF) >> 0;

        const uint8_t d3 = (opcode & 0xF000) >> 12;
        const uint8_t d2 = (opcode & 0x0F00) >> 8;
        const uint8_t d1 = (opcode & 0x00F0) >> 4;
        const uint8_t d0 = (opcode & 0x000F) >> 0;
        
        switch (d3)
        {
            case 0x0: {
                if (lsb == 0xE0) {
                    display->Clear();
                } // Clears the screen.
                else if (lsb == 0xEE) {
                    assert(memory->mem_interp_data.stack_level > 0);
                    memory->mem_interp_data.PC = *(uint16_t*)&memory->mem_stack[--memory->mem_interp_data.stack_level];
                } // Returns from a subroutine.
                else {
                    assert(false);
                } // Calls machine code routine at address NNN. Not necessary for most ROMs.
            } break;
            case 0x1: {
                memory->mem_interp_data.PC = opcode & 0x0FFF;
            } break; // Jumps to address NNN.
            case 0x2: {
                ((uint16_t*)&memory->mem_stack[0])[memory->mem_interp_data.stack_level++] = memory->mem_interp_data.PC;
            } break; // Calls subroutine at NNN.
            case 0x3: {
                const uint8_t X  = d2;
                const uint8_t NN = lsb;
                memory->mem_interp_data.PC++;
                if (registers->V[X] == NN) {
                    memory->mem_interp_data.PC++;
                }
            } break; // Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block);
            case 0x4: {
                const uint8_t X  = d2;
                const uint8_t NN = lsb;
                memory->mem_interp_data.PC++;
                if (registers->V[X] != NN) {
                    memory->mem_interp_data.PC++;
                }
            } break; // Skips the next instruction if VX does not equal NN. (Usually the next instruction is a jump to skip a code block);
            case 0x5: {
                const uint8_t X = (opcode & 0x0F00) >> 8;
                const uint8_t Y = (opcode & 0x00F0) >> 4;
                memory->mem_interp_data.PC++;
                if (registers->V[X] == registers->V[Y]) {
                    memory->mem_interp_data.PC++;
                }
            } break; // Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block);
            case 0x6: {
                const uint8_t  X  = (opcode & 0x0F00) >> 8;
                const uint8_t NN = opcode & 0x00FF;
                registers->V[X] = NN;
                memory->mem_interp_data.PC++;
            } break; // Sets VX to NN.
            case 0x7: {
                const uint8_t  X  = (opcode & 0x0F00) >> 8;
                const uint16_t NN = opcode & 0x00FF;
                registers->V[X] += NN;
                memory->mem_interp_data.PC++;
            } break; // Adds NN to VX. (Carry flag is not changed);
            case 0x8: {
                uint8_t X = d2;
                uint8_t Y = d1;
                switch(d0) {
                    case 0x0: {
                        registers->V[X] = registers->V[Y];
                    } break; // Sets VX to the value of VY.
                    case 0x1: {
                        registers->V[X] = registers->V[X] | registers->V[Y];
                    } break; // Sets VX to VX or VY. (Bitwise OR operation);
                    case 0x2: {
                        registers->V[X] = registers->V[X] & registers->V[Y];
                    } break; // Sets VX to VX and VY. (Bitwise AND operation);
                    case 0x3: {
                        registers->V[X] = registers->V[X] ^ registers->V[Y];
                    } break; // Sets VX to VX xor VY.
                    case 0x4: {
                        registers->VF = ((int)registers->V[X] + (int)registers->V[Y]) > 255 ? 1 : 0;
                        registers->V[X] += registers->V[Y];
                    } break; // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
                    case 0x5: {
                        registers->VF = ((int)registers->V[X] - (int)registers->V[Y]) < 0 ? 0 : 1;
                    } break; // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                    case 0x6: {
                        registers->VF = registers->V[X] & 0b00000001;
                        registers->V[X] >>= 1;
                    } break; // Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    case 0x7: {
                        registers->VF = ((int)registers->V[X] - (int)registers->V[Y]) < 0 ? 0 : 1;
                        registers->V[X] -= registers->V[Y];
                    } break; // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                    case 0xE: {
                        registers->VF = registers->V[X] & 0b10000000;
                        registers->V[X] <<= 1;
                    } break; // Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    default: { assert(false); } break;
                }
                memory->mem_interp_data.PC++;
            } break;
            case 0x9: {
                uint8_t X = d2;
                uint8_t Y = d1;
                memory->mem_interp_data.PC++;
                if (registers->V[X] == registers->V[Y]) {
                    memory->mem_interp_data.PC++;
                }
            } break; // Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block);
            case 0xA: {
                uint16_t NNN = opcode & 0x0FFF;
                registers->I = NNN;
            } break; // Sets I to the address NNN.
            case 0xB: {
                uint16_t NNN = opcode & 0x0FFF;
                memory->mem_interp_data.PC = NNN + registers->V[0];
            } break; // Jumps to the address NNN plus V0.
            case 0xC: {
                uint16_t X  = (opcode & 0x0F00) >> 8;
                uint16_t NN = opcode & 0x00FF;
                registers->V[X] = NN & GenRandomNumber();
            } break; // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
            case 0xD: {
                uint8_t X = (0x0F00) >> 8;
                uint8_t Y = (0x00F0) >> 4;
                uint8_t N = (0x000F) >> 0;
                display->DrawSprite(X, Y, N);
            } break; // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
            case 0xE: {
                if (lsb == 0x9E) {} // Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block);
                else if (lsb == 0xA1) {} // Skips the next instruction if the key stored in VX is not pressed. (Usually the next instruction is a jump to skip a code block);
                else { assert(false); }
            } break;
            case 0xF: {
                switch (lsb) {
                    case 0x07: {} break; // Sets VX to the value of the delay timer.
                    case 0x0A: {} break; // A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event);
                    case 0x15: {} break; // Sets the delay timer to VX.
                    case 0x18: {} break; // Sets the sound timer to VX.
                    case 0x1E: {} break; // Adds VX to I. VF is not affected.[c]
                    case 0x29: {} break; // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    case 0x33: {} break; // Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.);
                    case 0x55: {} break; // Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.[d]
                    case 0x65: {} break; // Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified.[d]
                    default: { assert(false); } break;
                }
            } break;

            default: { 
                assert(false); 
            } break;
        }
    }  
}