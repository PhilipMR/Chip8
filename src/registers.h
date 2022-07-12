#pragma once 

#include <stdint.h>
#include <array>
#include <bitset>

namespace ch8
{
    struct RegisterUnit
    {
        std::bitset<12> I;
        union {
            std::array<uint8_t, 16> V;
            struct {
                uint8_t V0;
                uint8_t V1;
                uint8_t V2;
                uint8_t V3;
                uint8_t V4;
                uint8_t V5;
                uint8_t V6;
                uint8_t V7;
                uint8_t V8;
                uint8_t V9;
                uint8_t VA;
                uint8_t VB;
                uint8_t VC;
                uint8_t VD;
                uint8_t VE;
                uint8_t VF;
            };
        };
    };
}