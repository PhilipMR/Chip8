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
                    uint16_t PC;
                    uint8_t  stack_level;
                    std::array<std::byte, MEMORY_INTERP-3> padding;
                } mem_interp_data;
            };
            std::array<std::byte, MEMORY_PRORAM>  mem_program;
            std::array<std::byte, MEMORY_STACK>   mem_stack;
            std::array<std::byte, MEMORY_DISPLAY> mem_display;
        };
    };
}