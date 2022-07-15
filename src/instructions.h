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

    static constexpr std::pair<uint8_t, SDL_Scancode> Chip8Keys[] = {
        { 0, SDL_SCANCODE_KP_0 },
        { 1, SDL_SCANCODE_KP_1 },
        { 2, SDL_SCANCODE_KP_2 },
        { 3, SDL_SCANCODE_KP_3 },
        { 4, SDL_SCANCODE_KP_4 },
        { 5, SDL_SCANCODE_KP_5 },
        { 6, SDL_SCANCODE_KP_6 },
        { 7, SDL_SCANCODE_KP_7 },
        { 8, SDL_SCANCODE_KP_8 },
        { 9, SDL_SCANCODE_KP_9 },
        { 10, SDL_SCANCODE_A },
        { 11, SDL_SCANCODE_B },
        { 12, SDL_SCANCODE_C },
        { 13, SDL_SCANCODE_D },
        { 14, SDL_SCANCODE_E },
        { 15, SDL_SCANCODE_F }
    };

    static bool 
    IsKeyPressed(uint8_t key)
    {
        const Uint8* kstate = SDL_GetKeyboardState(nullptr);
        return kstate[Chip8Keys[key].second];
    }

    static uint8_t 
    BlockUntilKeyPress()
    {
        while (true) {
            const Uint8* keys = SDL_GetKeyboardState(nullptr);
            for (auto k : Chip8Keys) {
                if (keys[k.second]) return k.first;
            }
            SDL_Delay(10);
        }
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
                if (opcode == 0x00E0) {
                    display->Clear();
                    memory->mem_interp_data.PC += 2;
                } // Clears the screen.
                else if (opcode == 0x00EE) {
                    assert(memory->mem_interp_data.SP > 0);
                    memory->mem_interp_data.SP--;
                    memory->mem_interp_data.PC = ((uint16_t*)&memory->mem_stack[0])[memory->mem_interp_data.SP];
                    memory->mem_interp_data.PC += 2;
                } // Returns from a subroutine.
                else {
                    assert(false);
                } // Calls machine code routine at address NNN. Not necessary for most ROMs.
            } break;
            case 0x1: {
                memory->mem_interp_data.PC = opcode & 0x0FFF;
            } break; // Jumps to address NNN.
            case 0x2: {
                uint16_t NNN = opcode & 0x0FFF;
                ((uint16_t*)&memory->mem_stack[0])[memory->mem_interp_data.SP] = memory->mem_interp_data.PC;
                memory->mem_interp_data.SP++;
                memory->mem_interp_data.PC = NNN;
            } break; // Calls subroutine at NNN.
            case 0x3: {
                const uint8_t X  = d2;
                const uint8_t NN = lsb;
                memory->mem_interp_data.PC += 2;
                if (registers->V[X] == NN) {
                    memory->mem_interp_data.PC += 2;
                }
            } break; // Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block);
            case 0x4: {
                const uint8_t X  = d2;
                const uint8_t NN = lsb;
                memory->mem_interp_data.PC += 2;
                if (registers->V[X] != NN) {
                    memory->mem_interp_data.PC += 2;
                }
            } break; // Skips the next instruction if VX does not equal NN. (Usually the next instruction is a jump to skip a code block);
            case 0x5: {
                assert(d0 == 0); // Only support CHIP-8 instruction set
                const uint8_t X = (opcode & 0x0F00) >> 8;
                const uint8_t Y = (opcode & 0x00F0) >> 4;
                memory->mem_interp_data.PC += 2;
                if (registers->V[X] == registers->V[Y]) {
                    memory->mem_interp_data.PC += 2;
                }
            } break; // Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block);
            case 0x6: {
                const uint8_t X  = (opcode & 0x0F00) >> 8;
                const uint8_t NN = opcode & 0x00FF;
                registers->V[X]  = NN;
                memory->mem_interp_data.PC += 2;
            } break; // Sets VX to NN.
            case 0x7: {
                const uint8_t  X  = (opcode & 0x0F00) >> 8;
                const uint16_t NN = opcode & 0x00FF;
                registers->V[X] += NN;
                memory->mem_interp_data.PC += 2;
            } break; // Adds NN to VX. (Carry flag is not changed);
            case 0x8: {
                uint8_t X = d2;
                uint8_t Y = d1;
                switch(d0) {
                    case 0x0: {
                        registers->V[X] = registers->V[Y];
                    } break; // Sets VX to the value of VY.
                    case 0x1: {
                        registers->V[X] |= registers->V[Y];
                    } break; // Sets VX to VX or VY. (Bitwise OR operation);
                    case 0x2: {
                        registers->V[X] &= registers->V[Y];
                    } break; // Sets VX to VX and VY. (Bitwise AND operation);
                    case 0x3: {
                        registers->V[X] ^= registers->V[Y];
                    } break; // Sets VX to VX xor VY.
                    case 0x4: {
                        registers->VF = ((int)registers->V[X] + (int)registers->V[Y]) > 255 ? 1 : 0;
                        registers->V[X] += registers->V[Y];
                    } break; // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
                    case 0x5: {
                        registers->VF = ((int)registers->V[X] - (int)registers->V[Y]) < 0 ? 0 : 1;
                        registers->V[X] -= registers->V[Y];
                    } break; // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                    case 0x6: {
                        registers->VF = (registers->V[X] & 0b00000001) > 0 ? 1 : 0;
                        registers->V[X] >>= 1;
                    } break; // Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    case 0x7: {
                        registers->VF = ((int)registers->V[Y] - (int)registers->V[X]) < 0 ? 0 : 1;
                        registers->V[X] = registers->V[Y] - registers->V[X];
                    } break; // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                    case 0xE: {
                        registers->VF = (registers->V[X] & 0b10000000) > 0 ? 1 : 0;
                        registers->V[X] <<= 1;
                    } break; // Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    default: { assert(false); } break;
                }
                memory->mem_interp_data.PC += 2;
            } break;
            case 0x9: {
                assert(d0 == 0); // Only support CHIP-8 instruction set
                uint8_t X = d2;
                uint8_t Y = d1;
                memory->mem_interp_data.PC += 2;
                if (registers->V[X] != registers->V[Y]) {
                    memory->mem_interp_data.PC += 2;
                }
            } break; // Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block);
            case 0xA: {
                uint16_t NNN = opcode & 0x0FFF;
                registers->I = NNN;
                memory->mem_interp_data.PC += 2;
            } break; // Sets I to the address NNN.
            case 0xB: {
                assert(((opcode & 0x0F00) >> 8) == 0); // Only support CHIP-8 instruction set
                uint16_t NNN = opcode & 0x0FFF;
                memory->mem_interp_data.PC = NNN + registers->V[0];
            } break; // Jumps to the address NNN plus V0.
            case 0xC: {
                uint16_t X  = (opcode & 0x0F00) >> 8;
                uint16_t NN = opcode & 0x00FF;
                registers->V[X] = NN & GenRandomNumber();
                memory->mem_interp_data.PC += 2;
            } break; // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
            case 0xD: {
                uint8_t X = (opcode & 0x0F00) >> 8;
                uint8_t Y = (opcode & 0x00F0) >> 4;
                uint8_t N = (opcode & 0x000F) >> 0;
                assert(N != 0); // Only support CHIP-8 instruction set
                display->DrawSprite(X, Y, N, registers, memory);
                memory->mem_interp_data.PC += 2;
            } break; // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
            case 0xE: {
                uint16_t X = (opcode & 0x0F00) >> 8;
                if (lsb == 0x9E) {
                    if (IsKeyPressed(registers->V[X])) {
                        memory->mem_interp_data.PC += 2;    
                    }
                    memory->mem_interp_data.PC += 2;
                } // Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block);
                else if (lsb == 0xA1) {
                    if (!IsKeyPressed(registers->V[X])) {
                        memory->mem_interp_data.PC += 2;    
                    }
                    memory->mem_interp_data.PC += 2;
                } // Skips the next instruction if the key stored in VX is not pressed. (Usually the next instruction is a jump to skip a code block);
                else { 
                    assert(false); 
                }
            } break;
            case 0xF: {
                uint16_t X = (opcode & 0x0F00) >> 8;
                switch (lsb) {
                    case 0x07: {
                        registers->V[X] = memory->mem_interp_data.delay_timer;
                    } break; // Sets VX to the value of the delay timer.
                    case 0x0A: {
                        registers->V[X] = BlockUntilKeyPress();
                    } break; // A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event);
                    case 0x15: {
                        memory->mem_interp_data.delay_timer = registers->V[X];
                    } break; // Sets the delay timer to VX.
                    case 0x18: {
                        memory->mem_interp_data.sound_timer = registers->V[X];
                    } break; // Sets the sound timer to VX.
                    case 0x1E: {
                        registers->I += registers->V[X];
                        registers->V[0xF] = registers->I > 0x0FFF ? 1 : 0; // Wiki says VF not affected, trapexit/chip8docs says it is...
                    } break; // Adds VX to I. VF is not affected.[c]
                    case 0x29: {
                        registers->I = memory->GetAddressOfCharSprite(registers->V[X]);
                    } break; // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    case 0x33: {
                        uint8_t vx = registers->V[X];
                        uint8_t hundreds = 0, tens = 0, ones = 0;
                        if (vx >= 100) {
                            hundreds = (vx - (vx % 100)) / 100;
                        }
                        if ((vx-hundreds) >= 10) {
                            tens = vx - hundreds;
                            tens = (tens - (tens % 10)) / 10;
                        }
                        ones = vx - (hundreds*100 + tens*10);
                        memory->memory[registers->I+0] = (std::byte)hundreds;
                        memory->memory[registers->I+1] = (std::byte)tens;
                        memory->memory[registers->I+2] = (std::byte)ones;
                    } break; // Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.);
                    case 0x55: {
                        for (uint16_t i = 0; i <= X; ++i) {
                            memory->memory[registers->I + i] = (std::byte)registers->V[i];
                        }
                    } break; // Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.[d]
                    case 0x65: {
                        for (uint16_t i = 0; i <= X; ++i) {
                            registers->V[i] = (uint8_t)memory->memory[registers->I + i];
                        }
                    } break; // Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified.[d]
                    default: { 
                        assert(false); 
                    } break;
                }
                memory->mem_interp_data.PC += 2;
            } break;

            default: { 
                assert(false); 
            } break;
        }
    }  
}