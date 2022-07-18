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
    constexpr int  WINDOW_HEIGHT  = 320;

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

        //SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        if (m_renderer == nullptr) {
            throw std::runtime_error("Could not create the SDL Renderer");
        }

        for (int y = 0; y < RESOLUTION_Y; ++y) {
            for (int x = 0; x < RESOLUTION_X; ++x) {
                Pixel pixel = {0};
                pixel.rect = { x * PIXELSIZE_X, y * PIXELSIZE_Y, PIXELSIZE_X, PIXELSIZE_Y };
                pixel.is_on = false;
                m_pixels[y*RESOLUTION_X+x] = pixel;
            }
        }

        m_debugFont = TTF_OpenFont("D:\\Projects\\Chip-8\\OpenSans.ttf", 12);
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
        for (auto& p : m_pixels) {
            p.is_on = false;
        }
        m_dirty = true;
    }

    void
    DisplayUnit::DrawSprite(uint8_t X, uint8_t Y, uint8_t N, RegisterUnit* registers, const MemoryUnit* memory)
    {
        uint16_t I = registers->I;
        bool collision = false;
        for (int y = 0; y < N; ++y) {
            uint8_t row_bits = (uint8_t)memory->memory[I+y];
            for (int x = 0; x < 8; ++x) {
                if (!(row_bits & (0b10000000 >> x))) continue;
                auto& pixel = m_pixels[((Y+y)%RESOLUTION_Y)*RESOLUTION_X+((X+x)%RESOLUTION_X)];
                if (pixel.is_on) {
                    collision = true;
                }
                pixel.is_on ^= 1;
            }
        }
        registers->V[0xF] = collision ? 1 : 0;
        m_dirty = true;
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


        int x_offset = WINDOW_WIDTH - 100;

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

        SDL_Rect registers_rect{ x_offset, 0, col1_width+col2_width+2*padding_x+50, font_height*19};
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 0);
        SDL_RenderDrawRect(m_renderer, &registers_rect);

        SDL_Rect interp_rect{ x_offset, 0, col1_width+col2_width+2*padding_x+50, font_height*2};
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 0);
        SDL_RenderDrawRect(m_renderer, &interp_rect);
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);

        {
            int w;
            const char* text = "PC";
            TTF_SizeText(m_debugFont, text, &w, nullptr);
            DrawText(text, x_offset, pen_y, w, font_height);

            int pen_x = x_offset + col1_width + padding_x;
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
            DrawText(ss.str().c_str(), x_offset, pen_y, w, font_height);

            int pen_x = x_offset + col1_width + padding_x;
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
            DrawText(ss.str().c_str(), x_offset, pen_y, w, font_height);

            int pen_x = x_offset + col1_width + padding_x;
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
            DrawText(ss.str().c_str(), x_offset, pen_y, w, font_height);

            int pen_x = x_offset + col1_width + padding_x;
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
    DisplayUnit::Present(const RegisterUnit* registers, const MemoryUnit* memory)
    {
        if (!m_dirty) return;
        m_dirty = false;
        SDL_RenderClear(m_renderer);
        for (const auto& pixel : m_pixels) {
            const SDL_Color color = { 
                Uint8(pixel.is_on * 255), 
                Uint8(pixel.is_on * 255), 
                Uint8(pixel.is_on * 255), 
                255 
            };
            SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(m_renderer, &pixel.rect);
        }
        //DrawDebugInfo(registers, memory);
        SDL_RenderPresent(m_renderer);
    }
}