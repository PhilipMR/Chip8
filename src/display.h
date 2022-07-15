#pragma once 

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace ch8
{
    struct RegisterUnit;
    union MemoryUnit;
    class DisplayUnit
    {
    public:
        static constexpr int RESOLUTION_X = 64;
        static constexpr int RESOLUTION_Y = 32;

    private:
        TTF_Font*     m_debugFont;
        SDL_Window*   m_window;
        SDL_Renderer* m_renderer;
        struct Pixel {
            SDL_Rect rect;
            Uint8    is_on;
        } m_pixels[RESOLUTION_Y*RESOLUTION_X];

        void DrawDebugInfo(const RegisterUnit* registers, const MemoryUnit* memory);
    public:
        DisplayUnit();
        ~DisplayUnit();
        void Clear();
        void DrawSprite(uint8_t X, uint8_t Y, uint8_t N, RegisterUnit* registers, const MemoryUnit* memory);
        void Present(const RegisterUnit* registers, const MemoryUnit* memory);
    };
}