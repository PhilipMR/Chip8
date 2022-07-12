#pragma once 

#include <vector>
#include <cstddef>
#include <span>

#include "display.h"
#include "memory.h"
#include "registers.h"

namespace ch8 
{
    class GameROM {
        std::vector<std::byte> m_data;
    public:
        GameROM(const char* path);
        std::span<const std::byte> GetData() const;
    };

    class Chip8Emulator
    {
        DisplayUnit     m_display;
        MemoryUnit      m_memory;
        RegisterUnit    m_registers;
    public:
        Chip8Emulator();
        explicit Chip8Emulator(const GameROM* rom);
        void LoadROM(const GameROM* rom);
        void Run();
    }; 
}