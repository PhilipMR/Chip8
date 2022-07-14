#include "display.h"
#include "registers.h"
#include "memory.h"

#include <stdexcept>
#include <iomanip>
#include <sstream>

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

        m_debugFont = TTF_OpenFont("C:\\Users\\phili\\Desktop\\Chip8\\OpenSans.ttf", 12);
        TTF_SetFontHinting(m_debugFont, TTF_HINTING_LIGHT_SUBPIXEL);
        if (m_debugFont == nullptr) {
            throw std::runtime_error("Could not open the debug font");
        }
    }

    DisplayUnit::~DisplayUnit()
    {
        TTF_CloseFont(m_debugFont);
    }

    void 
    DisplayUnit::Clear() 
    {
        SDL_RenderClear(m_renderer);
    }

    void
    DisplayUnit::DrawSprite(uint8_t X, uint8_t Y, uint8_t N)
    {

    }

    void 
    DisplayUnit::DrawDebugInfo(const RegisterUnit* registers, const MemoryUnit* memory)
    {
        auto GetHexDigit = [](int digit) -> char {
            if (digit <= 9) return '0' + digit;
            else return 'A' + (digit - 10);
        };

        auto DrawText = [&](const char* text, int pen_x, int pen_y, int w, int h) -> void {
            constexpr SDL_Color white = {255, 255, 255};

            SDL_Surface* surface_msg = TTF_RenderText_Solid(m_debugFont, text, white);
            SDL_Texture* msg = SDL_CreateTextureFromSurface(m_renderer, surface_msg);
            SDL_Rect msg_rect = { pen_x, pen_y, w, h };
            SDL_RenderCopy(m_renderer, msg, nullptr, &msg_rect);
            SDL_FreeSurface(surface_msg);
            SDL_DestroyTexture(msg);
        };


        int col1_width = 0;
        for (int i = 0; i < 16; ++i) {
            std::stringstream ss;
            ss << "V" << GetHexDigit(i);

            int w;
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            if (w > col1_width) col1_width = w;
        }

        int col2_width = 0;
        TTF_SizeText(m_debugFont, "=", &col2_width, nullptr);

        int padding_x = 5;
        int font_height = TTF_FontHeight(m_debugFont);
        int pen_y = 0;

        SDL_Rect registers_rect{ 0, 0, col1_width+col2_width+2*padding_x+50, font_height*19};
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 0);
        SDL_RenderDrawRect(m_renderer, &registers_rect);

        SDL_Rect interp_rect{ 0, 0, col1_width+col2_width+2*padding_x+50, font_height*2};
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 0);
        SDL_RenderDrawRect(m_renderer, &interp_rect);
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);

        {
            int w;
            const char* text = "PC";
            TTF_SizeText(m_debugFont, text, &w, nullptr);
            DrawText(text, 0, pen_y, w, font_height);

            int pen_x = col1_width + padding_x;
            DrawText("=", pen_x, pen_y, col2_width, font_height);
            pen_x += col2_width + padding_x;

            std::stringstream ss;
            ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << (int)memory->mem_interp_data.PC;
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), pen_x, pen_y, w, font_height);

            pen_y += font_height;
        }
        {
            int w;
            std::stringstream ss;
            ss << "OP";
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), 0, pen_y, w, font_height);

            int pen_x = col1_width + padding_x;
            DrawText("=", pen_x, pen_y, col2_width, font_height);
            pen_x += col2_width + padding_x;

            uint16_t opcode = *(uint16_t*)(&memory->memory[memory->mem_interp_data.PC]);
            opcode = ((opcode & 0x00FF) << 8) | ((opcode & 0xFF00) >> 8); // Switch endianness
            ss.str(std::string());
            ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << (int)opcode;
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), pen_x, pen_y, w, font_height);

            pen_y += font_height;
        }
        {
            int w;
            std::stringstream ss;
            ss << " I";
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), 0, pen_y, w, font_height);

            int pen_x = col1_width + padding_x;
            DrawText("=", pen_x, pen_y, col2_width, font_height);
            pen_x += col2_width + padding_x;

            ss.str(std::string());
            ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << (int)registers->I;
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), pen_x, pen_y, w, font_height);

            pen_y += font_height;
        }
        for (int i = 0; i < 16; ++i) {
            int w;
            std::stringstream ss;
            ss << "V" << GetHexDigit(i);
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), 0, pen_y, w, font_height);

            int pen_x = col1_width + padding_x;
            DrawText("=", pen_x, pen_y, col2_width, font_height);
            pen_x += col2_width + padding_x;

            ss.str(std::string());
            ss << "0x" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)registers->V[i];
            TTF_SizeText(m_debugFont, ss.str().c_str(), &w, nullptr);
            DrawText(ss.str().c_str(), pen_x, pen_y, w, font_height);

            pen_y += font_height;
        }
    }

    void 
    DisplayUnit::Present()
    {
//        for (int y = 0; y < RESOLUTION_Y; ++y) {
//            for (int x = 0; x < RESOLUTION_X; ++x) {
//                const Pixel& pixel = m_pixels[y][x];
//                SDL_SetRenderDrawColor(m_renderer, pixel.r, pixel.g, pixel.b, 255);
//                SDL_RenderFillRect(m_renderer, &pixel.rect);
//            }
//        }
        SDL_RenderPresent(m_renderer);
    }
}