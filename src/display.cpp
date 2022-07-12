#include "display.h"

#include <stdexcept>

namespace ch8 
{
    constexpr char WINDOW_TITLE[] = "Chip-8 Emulator";
    constexpr int  WINDOW_WIDTH   = 640;
    constexpr int  WINDOW_HEIGHT  = 480;

    constexpr int PIXELSIZE_X  = WINDOW_WIDTH  / DisplayUnit::RESOLUTION_X;
    constexpr int PIXELSIZE_Y  = WINDOW_HEIGHT / DisplayUnit::RESOLUTION_Y;

    DisplayUnit::DisplayUnit()
    {
        m_window = SDL_CreateWindow(WINDOW_TITLE,
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                    WINDOW_WIDTH, WINDOW_HEIGHT, 
                                    0);
        if (m_window == nullptr) {
            throw std::runtime_error("Could not create the SDL window");
        }

        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        if (m_renderer == nullptr) {
            throw std::runtime_error("Could not create the SDL Renderer");
        }

        for (int y = 0; y < RESOLUTION_Y; ++y) {
            for (int x = 0; x < RESOLUTION_X; ++x) {
                Pixel pixel = {0};
                pixel.rect = { x * PIXELSIZE_X, y * PIXELSIZE_Y, PIXELSIZE_X, PIXELSIZE_Y };
                m_pixels[y][x] = pixel;
            }
        } 
    }

    void 
    DisplayUnit::Clear() 
    {
        SDL_RenderClear(m_renderer);
    }
 
    void 
    DisplayUnit::DrawDebugInfo(const RegisterUnit* registers)
    {
    }

    void 
    DisplayUnit::Present()
    {
        for (int y = 0; y < RESOLUTION_Y; ++y) {
            for (int x = 0; x < RESOLUTION_X; ++x) {
                const Pixel& pixel = m_pixels[y][x];
                SDL_SetRenderDrawColor(m_renderer, pixel.r, pixel.g, pixel.b, 255);
                SDL_RenderFillRect(m_renderer, &pixel.rect);
            }
        }
        SDL_RenderPresent(m_renderer);
    }
}