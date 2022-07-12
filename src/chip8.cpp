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
    }

    Chip8Emulator::Chip8Emulator(const GameROM* rom)
    {
        m_memory.memory = {};
        LoadROM(rom);
    }

    void 
    Chip8Emulator::LoadROM(const GameROM* rom)
    {
        auto rom_data = rom->GetData();
        std::copy(rom_data.begin(), rom_data.end(), m_memory.mem_program_8.begin());
        m_memory.mem_interp_data.PC = MEMORY_INTERP;
        m_memory.mem_interp_data.stack_level = 0;
    }

    void 
    Chip8Emulator::Run()
    {
        bool is_running = true;
        while (is_running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) 
            {
                if (event.type == SDL_QUIT) {
                    is_running = false;
                }
            }

            uint16_t opcode = m_memory.mem_program[m_memory.mem_interp_data.PC - 512];
            opcode = ((opcode & 0x00FF) << 8) | ((opcode & 0xFF00) >> 8); // Switch endianness
            ProcessOpCode(opcode, &m_display, &m_registers, &m_memory);

            //m_display.DrawDebugInfo(&m_registers);
            m_display.Present();
        }
    }
}