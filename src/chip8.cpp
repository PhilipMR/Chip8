#include "chip8.h"

#include <fstream>
#include <stdexcept>
#include <span>

#include "instructions.h"

namespace ch8
{
    GameROM::GameROM(const char* path)
    {
        std::ifstream file(path, std::ifstream::in | std::ifstream::binary | std::ifstream::ate);
        if (!file.is_open()) {
            throw std::runtime_error(std::string{"Could not open ROM at path: "} + path);
        }
        auto sz = file.tellg();
        file.seekg(SEEK_SET, 0);
        m_data.resize(sz);
        file.read((char*)&m_data[0], sz);
    }

    std::span<const std::byte> 
    GameROM::GetData() const
    {
        return {&m_data[0], m_data.size()};
    }



    Chip8Emulator::Chip8Emulator()
    {
        m_memory.memory = {};
        m_registers.I = 0;
        for (auto& v : m_registers.V) {
            v = 0;
        }
    }

    Chip8Emulator::Chip8Emulator(const GameROM* rom)
    {
        LoadROM(rom);
    }

    void 
    Chip8Emulator::LoadROM(const GameROM* rom)
    {
        m_registers.I = 0;
        for (auto& v : m_registers.V) {
            v = 0;
        }
        auto rom_data = rom->GetData();
        std::copy(rom_data.begin(), rom_data.end(), m_memory.mem_program.begin());
        m_memory.mem_interp_data.PC = (uint16_t)(&m_memory.mem_program[0] - &m_memory.memory[0]);
        m_memory.mem_interp_data.SP = 0;
    }

    void 
    Chip8Emulator::Run()
    {
        bool is_running = true;
        bool ready_for_next_instr = false;
        while (is_running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) 
            {
                if (event.type == SDL_QUIT) {
                    is_running = false;
                }
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDL_KeyCode::SDLK_RIGHT) {
                        ready_for_next_instr = true;
                    }
                }
            }

            //SDL_Delay(8);
            ready_for_next_instr = true;
            if (ready_for_next_instr) {
                uint16_t opcode = *(uint16_t*)(&m_memory.memory[m_memory.mem_interp_data.PC]);
                opcode = ((opcode & 0x00FF) << 8) | ((opcode & 0xFF00) >> 8); // Switch endianness
                ProcessOpCode(opcode, &m_display, &m_registers, &m_memory);
                ready_for_next_instr = false;
            }

            static Uint64 last_timer_update = 0;
            Uint64 time = SDL_GetTicks64();
            if ((time-last_timer_update) >= (1000.0/60.0)) {
                if (m_memory.mem_interp_data.delay_timer > 0) {
                    m_memory.mem_interp_data.delay_timer--;
                }
                if (m_memory.mem_interp_data.sound_timer > 0) {
                    m_memory.mem_interp_data.sound_timer--;
                }
                last_timer_update = time;
            }

            m_display.Present(&m_registers, &m_memory);
        }
    }
}