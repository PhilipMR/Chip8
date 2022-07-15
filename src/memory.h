#pragma once 

#include <cstddef>
#include <array>

namespace ch8
{
    static constexpr size_t MEMORY_TOTAL   = 4096;
    static constexpr size_t MEMORY_INTERP  = 512;
    static constexpr size_t MEMORY_STACK   = 96;
    static constexpr size_t MEMORY_DISPLAY = 256;
    static constexpr size_t MEMORY_PRORAM  = MEMORY_TOTAL - (MEMORY_INTERP + MEMORY_STACK + MEMORY_DISPLAY);

    union MemoryUnit
    {
        std::array<std::byte, MEMORY_TOTAL> memory;
        struct {
            union {
                std::array<std::byte, MEMORY_INTERP> mem_interp;
                struct {
                    std::array<uint8_t, 80> font;
                    uint16_t PC;
                    uint8_t  SP;
                    uint8_t  delay_timer;
                    uint8_t  sound_timer;
                } mem_interp_data;
            };
            std::array<std::byte, MEMORY_PRORAM>  mem_program;
            std::array<std::byte, MEMORY_STACK>   mem_stack;
            std::array<std::byte, MEMORY_DISPLAY> mem_display;
        };

        MemoryUnit()
        {
            this->memory = {};
            uint8_t font_data[] = {
                0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
                0x20, 0x60, 0x20, 0x20, 0x70,		// 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
                0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
                0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
                0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
                0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
                0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
                0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
                0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
                0xF0, 0x80, 0xF0, 0x80, 0x80		// F
            };
            memcpy(&mem_interp_data.font[0], font_data, sizeof(font_data));
        }

        inline constexpr uint16_t 
        GetAddressOfCharSprite(uint8_t c)
        {
            return c*5;
        };
    };
}