#pragma once 

#include <SDL2/SDL.h>

struct SDL_Window;
struct SDL_Renderer;
namespace ch8 
{
    struct RegisterUnit;

    class DisplayUnit
    {
    public:
        static constexpr int RESOLUTION_X = 64;
        static constexpr int RESOLUTION_Y = 32;

    private:
        SDL_Window*   m_window;
        SDL_Renderer* m_renderer;
        struct Pixel {
            SDL_Rect rect;
            Uint8    r, g, b;
        } m_pixels[RESOLUTION_Y][RESOLUTION_X];

    public:
        DisplayUnit();
        void Clear();
        void DrawDebugInfo(const RegisterUnit* registers);
        void Present();
    };
}