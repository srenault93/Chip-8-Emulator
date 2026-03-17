#include "SDL2/SDL.h"
#include "chip8.h"
#include <cstdio>
#include <cstring>
#include <cmath>

// ── Layout constants ──────────────────────────────────────────────────
static const int SCALE        = 10;
static const int DISPLAY_W    = 64 * SCALE;   // 640
static const int DISPLAY_H    = 32 * SCALE;   // 320

// Keypad panel sits below the display
static const int PAD_MARGIN   = 16;           // outer margin around keypad
static const int PAD_GAP      = 8;            // gap between keys
static const int KEY_W        = 70;           // key button width
static const int KEY_H        = 60;           // key button height
static const int PAD_COLS     = 4;
static const int PAD_ROWS     = 4;
static const int PAD_PANEL_H  = PAD_MARGIN * 2 + PAD_ROWS * KEY_H + (PAD_ROWS - 1) * PAD_GAP;
static const int STATUS_H     = 28;           // status bar height
static const int WINDOW_W     = DISPLAY_W;    // 640
static const int WINDOW_H     = DISPLAY_H + STATUS_H + PAD_PANEL_H; // 320+28+panel

// Speed limits
static const int CYCLES_MIN   = 1;
static const int CYCLES_MAX   = 30;
static const int CYCLES_DEF   = 8;

// ── Colours ───────────────────────────────────────────────────────────
struct Color { Uint8 r, g, b, a; };
static const Color C_DISPLAY_ON  = { 0x00, 0xFF, 0x00, 0xFF }; // bright green
static const Color C_DISPLAY_OFF = { 0x00, 0x1A, 0x00, 0xFF }; // dark green
static const Color C_PANEL_BG    = { 0x1A, 0x1A, 0x2E, 0xFF }; // dark navy
static const Color C_KEY_UP      = { 0x16, 0x21, 0x3E, 0xFF }; // key unpressed
static const Color C_KEY_DOWN    = { 0x00, 0xB4, 0xD8, 0xFF }; // key pressed (cyan)
static const Color C_KEY_BORDER  = { 0x0F, 0x3A, 0x6B, 0xFF }; // key border
static const Color C_STATUS_BG   = { 0x0D, 0x0D, 0x1A, 0xFF }; // status bar bg
static const Color C_STATUS_TEXT = { 0x00, 0xFF, 0x80, 0xFF }; // status text (unused without font)
static const Color C_PAUSED      = { 0xFF, 0xA5, 0x00, 0xFF }; // orange pause overlay

// ── SDL wrapper ───────────────────────────────────────────────────────
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
} sdl_t;

// ── CHIP-8 keypad layout (row-major, top-left to bottom-right) ────────
// Visual:  1 2 3 C       Keyboard:  1 2 3 4
//          4 5 6 D                  Q W E R
//          7 8 9 E                  A S D F
//          A 0 B F                  Z X C V
static const int KEY_ORDER[16] = {
    0x1, 0x2, 0x3, 0xC,
    0x4, 0x5, 0x6, 0xD,
    0x7, 0x8, 0x9, 0xE,
    0xA, 0x0, 0xB, 0xF
};
static const char KEY_LABELS[16][3] = {
    "1","2","3","C",
    "4","5","6","D",
    "7","8","9","E",
    "A","0","B","F"
};

// ── Helper: set SDL draw color ────────────────────────────────────────
static void set_color(SDL_Renderer *r, Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

// ── Helper: filled rect with border ──────────────────────────────────
static void draw_key_rect(SDL_Renderer *r, SDL_Rect rect, Color fill, Color border, bool pressed) {
    // shadow / depth effect when not pressed
    if (!pressed) {
        SDL_Rect shadow = { rect.x + 3, rect.y + 3, rect.w, rect.h };
        set_color(r, { 0x05, 0x05, 0x10, 0xFF });
        SDL_RenderFillRect(r, &shadow);
    }
    // fill
    set_color(r, fill);
    SDL_RenderFillRect(r, &rect);
    // border
    set_color(r, border);
    SDL_RenderDrawRect(r, &rect);
    // inner highlight on top edge when unpressed
    if (!pressed) {
        set_color(r, { (Uint8)SDL_min(fill.r + 40, 255),
                       (Uint8)SDL_min(fill.g + 40, 255),
                       (Uint8)SDL_min(fill.b + 40, 255), 0xFF });
        SDL_RenderDrawLine(r, rect.x+1, rect.y+1, rect.x+rect.w-2, rect.y+1);
    }
}

// ── Compute rect for keypad button i ─────────────────────────────────
static SDL_Rect key_rect(int i) {
    int col = i % PAD_COLS;
    int row = i / PAD_COLS;
    int panel_x = (WINDOW_W - (PAD_COLS * KEY_W + (PAD_COLS-1) * PAD_GAP)) / 2;
    int panel_y = DISPLAY_H + STATUS_H + PAD_MARGIN;
    return {
        panel_x + col * (KEY_W + PAD_GAP),
        panel_y + row * (KEY_H + PAD_GAP),
        KEY_W, KEY_H
    };
}

// ── Hit test: which keypad button was clicked? (-1 = none) ───────────
static int keypad_hit(int mx, int my) {
    for (int i = 0; i < 16; ++i) {
        SDL_Rect r = key_rect(i);
        if (mx >= r.x && mx < r.x+r.w && my >= r.y && my < r.y+r.h)
            return i;
    }
    return -1;
}

// ── Draw a single digit using line primitives (no font needed) ────────
// Draws at centre of (cx, cy) in the given colour, size ~20x28
static void draw_digit(SDL_Renderer *r, char ch, int cx, int cy, Color c) {
    set_color(r, c);
    // Segments: 7-segment style using SDL lines
    // Each char maps to a small set of horizontal/vertical lines
    const int W = 8, H = 12; // half-widths
    int x = cx - W/2, y = cy - H/2;

    // Horizontal segments: top, mid, bot
    auto hl = [&](int sx, int sy, int len) {
        SDL_RenderDrawLine(r, sx, sy, sx+len, sy);
        SDL_RenderDrawLine(r, sx, sy+1, sx+len, sy+1);
    };
    // Vertical segments
    auto vl = [&](int sx, int sy, int len) {
        SDL_RenderDrawLine(r, sx, sy, sx, sy+len);
        SDL_RenderDrawLine(r, sx+1, sy, sx+1, sy+len);
    };

    bool top=0,mid=0,bot=0,tl=0,tr=0,bl=0,br=0;
    switch(ch) {
        case '0': top=mid=0; top=1;bot=1;tl=1;tr=1;bl=1;br=1; mid=0; break;
        case '1': tr=1;br=1; break;
        case '2': top=1;tr=1;mid=1;bl=1;bot=1; break;
        case '3': top=1;tr=1;mid=1;br=1;bot=1; break;
        case '4': tl=1;tr=1;mid=1;br=1; break;
        case '5': top=1;tl=1;mid=1;br=1;bot=1; break;
        case '6': top=1;tl=1;mid=1;bl=1;br=1;bot=1; break;
        case '7': top=1;tr=1;br=1; break;
        case '8': top=1;tl=1;tr=1;mid=1;bl=1;br=1;bot=1; break;
        case '9': top=1;tl=1;tr=1;mid=1;br=1;bot=1; break;
        case 'A': top=1;tl=1;tr=1;mid=1;bl=1;br=1; break;
        case 'B': tl=1;mid=1;bl=1;br=1;bot=1; break;
        case 'C': top=1;tl=1;bl=1;bot=1; break;
        case 'D': tr=1;mid=1;bl=1;br=1;bot=1; break;  // approx
        case 'E': top=1;tl=1;mid=1;bl=1;bot=1; break;
        case 'F': top=1;tl=1;mid=1;bl=1; break;
        default: break;
    }
    if(top) hl(x,   y,       W);
    if(mid) hl(x,   y+H/2,   W);
    if(bot) hl(x,   y+H,     W);
    if(tl)  vl(x,   y,       H/2);
    if(tr)  vl(x+W, y,       H/2);
    if(bl)  vl(x,   y+H/2,   H/2);
    if(br)  vl(x+W, y+H/2,   H/2);
}

// ── Draw status bar ───────────────────────────────────────────────────
static void draw_status(SDL_Renderer *r, bool paused, int cycles, const char *rom) {
    // Background
    SDL_Rect bar = { 0, DISPLAY_H, WINDOW_W, STATUS_H };
    set_color(r, C_STATUS_BG);
    SDL_RenderFillRect(r, &bar);

    // Separator line
    set_color(r, C_KEY_BORDER);
    SDL_RenderDrawLine(r, 0, DISPLAY_H, WINDOW_W, DISPLAY_H);
    SDL_RenderDrawLine(r, 0, DISPLAY_H + STATUS_H - 1, WINDOW_W, DISPLAY_H + STATUS_H - 1);

    // Draw small indicator blocks for: PAUSED | SPEED | keys hint
    int cx = 12, cy = DISPLAY_H + STATUS_H/2;

    // PAUSED indicator
    Color pause_col = paused ? C_PAUSED : Color{0x33,0x33,0x44,0xFF};
    SDL_Rect pi = { cx-10, cy-5, 20, 10 };
    set_color(r, pause_col);
    SDL_RenderFillRect(r, &pi);
    // Draw "P" indicator
    set_color(r, paused ? Color{0,0,0,255} : Color{0x55,0x55,0x66,0xFF});
    SDL_RenderDrawLine(r, cx-4, cy-4, cx-4, cy+4);
    SDL_RenderDrawLine(r, cx-4, cy-4, cx+2, cy-4);
    SDL_RenderDrawLine(r, cx+2, cy-4, cx+2, cy);
    SDL_RenderDrawLine(r, cx-4, cy,   cx+2, cy);

    // Speed display: draw small squares representing speed level
    int sx = 40;
    set_color(r, C_KEY_BORDER);
    for (int i = 0; i < CYCLES_MAX; i += 2) {
        SDL_Rect sq = { sx + i*3, cy - 4, 4, 8 };
        if ((i/2)*2 < cycles)
            set_color(r, C_DISPLAY_ON);
        else
            set_color(r, Color{0x22,0x22,0x33,0xFF});
        SDL_RenderFillRect(r, &sq);
    }

    // Key hints on right side (tiny coloured pills)
    struct { const char* label; Color col; int x; } hints[] = {
        { "P:PAUSE", {0xFF,0xA5,0x00,0xFF}, WINDOW_W - 260 },
        { "R:RESET", {0x00,0xFF,0x80,0xFF}, WINDOW_W - 180 },
        { "+/-:SPD",  {0x00,0xB4,0xD8,0xFF}, WINDOW_W - 100 },
    };
    for (auto &h : hints) {
        SDL_Rect pill = { h.x, cy-6, 74, 12 };
        set_color(r, Color{0x22,0x22,0x33,0xFF});
        SDL_RenderFillRect(r, &pill);
        set_color(r, h.col);
        SDL_RenderDrawRect(r, &pill);
        // draw each character of label
        const char *lp = h.label;
        int lx = h.x + 4;
        while (*lp) {
            draw_digit(r, *lp, lx+4, cy, h.col);
            lx += 10;
            lp++;
        }
    }
}

// ── Draw keypad panel ─────────────────────────────────────────────────
static void draw_keypad(SDL_Renderer *r, Chip8 &chip8) {
    // Panel background
    SDL_Rect panel = { 0, DISPLAY_H + STATUS_H, WINDOW_W, PAD_PANEL_H };
    set_color(r, C_PANEL_BG);
    SDL_RenderFillRect(r, &panel);

    for (int i = 0; i < 16; ++i) {
        int chip8_key = KEY_ORDER[i];
        bool pressed  = chip8.get_key(chip8_key);
        SDL_Rect rect = key_rect(i);

        Color fill   = pressed ? C_KEY_DOWN : C_KEY_UP;
        Color border = pressed ? C_DISPLAY_ON : C_KEY_BORDER;

        draw_key_rect(r, rect, fill, border, pressed);

        // Draw label digit centred in key
        Color label_col = pressed
            ? Color{0x00,0x00,0x10,0xFF}
            : Color{0x00,0xCC,0xFF,0xFF};
        draw_digit(r, KEY_LABELS[i][0], rect.x + rect.w/2, rect.y + rect.h/2, label_col);
    }
}

// ── Full redraw ───────────────────────────────────────────────────────
static void full_render(const sdl_t &sdl, Chip8 &chip8, bool paused, int cycles, const char *rom) {
    // Display area
    set_color(sdl.renderer, C_DISPLAY_OFF);
    SDL_Rect display_rect = {0, 0, DISPLAY_W, DISPLAY_H};
    SDL_RenderFillRect(sdl.renderer, &display_rect);

    set_color(sdl.renderer, C_DISPLAY_ON);
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x)
            if (chip8.get_pixel(x, y)) {
                SDL_Rect rect = { x*SCALE, y*SCALE, SCALE, SCALE };
                SDL_RenderFillRect(sdl.renderer, &rect);
            }

    // Pause overlay
    if (paused) {
        set_color(sdl.renderer, {0xFF, 0xA5, 0x00, 60});
        SDL_SetRenderDrawBlendMode(sdl.renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(sdl.renderer, &display_rect);
        SDL_SetRenderDrawBlendMode(sdl.renderer, SDL_BLENDMODE_NONE);
        // Draw two pause bars
        set_color(sdl.renderer, C_PAUSED);
        SDL_Rect bar1 = { DISPLAY_W/2 - 22, DISPLAY_H/2 - 24, 14, 48 };
        SDL_Rect bar2 = { DISPLAY_W/2 +  8, DISPLAY_H/2 - 24, 14, 48 };
        SDL_RenderFillRect(sdl.renderer, &bar1);
        SDL_RenderFillRect(sdl.renderer, &bar2);
    }

    draw_status(sdl.renderer, paused, cycles, rom);
    draw_keypad(sdl.renderer, chip8);
    SDL_RenderPresent(sdl.renderer);
    chip8.draw_flag = false;
}

// ── SDL init ──────────────────────────────────────────────────────────
bool init_SDL(sdl_t *sdl) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("%s", SDL_GetError()); return false;
    }
    sdl->window = SDL_CreateWindow("CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, 0);
    if (!sdl->window)   { SDL_Log("%s", SDL_GetError()); return false; }
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl->renderer) { SDL_Log("%s", SDL_GetError()); return false; }
    return true;
}

void cleanup(const sdl_t sdl) {
    SDL_DestroyRenderer(sdl.renderer);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit();
}

// ── Input handling ────────────────────────────────────────────────────
// Returns true if a full redraw is needed (keypad state changed)
static bool handle_input(Chip8 &chip8, bool &paused, int &cycles,
                         const char *rom_path, int &mouse_held_key) {
    bool needs_redraw = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                chip8.state = Chip8::QUIT;
                break;

            // ── Keyboard ──
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                bool pressed = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.sym) {
                    // Meta controls (keydown only)
                    case SDLK_ESCAPE: chip8.state = Chip8::QUIT; break;
                    case SDLK_p:
                        if (pressed) { paused = !paused; needs_redraw = true; }
                        break;
                    case SDLK_r:
                        if (pressed) {
                            chip8.initialize();
                            chip8.loadRom(rom_path);
                            paused = false;
                            needs_redraw = true;
                        }
                        break;
                    case SDLK_EQUALS: // + key
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        if (pressed && cycles < CYCLES_MAX) { cycles++; needs_redraw = true; }
                        break;
                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        if (pressed && cycles > CYCLES_MIN) { cycles--; needs_redraw = true; }
                        break;
                    // CHIP-8 keypad
                    case SDLK_1: chip8.set_key(0x1, pressed); needs_redraw=true; break;
                    case SDLK_2: chip8.set_key(0x2, pressed); needs_redraw=true; break;
                    case SDLK_3: chip8.set_key(0x3, pressed); needs_redraw=true; break;
                    case SDLK_4: chip8.set_key(0xC, pressed); needs_redraw=true; break;
                    case SDLK_q: chip8.set_key(0x4, pressed); needs_redraw=true; break;
                    case SDLK_w: chip8.set_key(0x5, pressed); needs_redraw=true; break;
                    case SDLK_e: chip8.set_key(0x6, pressed); needs_redraw=true; break;
                    // 'r' handled above as reset, not key 0xD — no conflict since R is reset
                    case SDLK_a: chip8.set_key(0x7, pressed); needs_redraw=true; break;
                    case SDLK_s: chip8.set_key(0x8, pressed); needs_redraw=true; break;
                    case SDLK_d: chip8.set_key(0x9, pressed); needs_redraw=true; break;
                    case SDLK_f: chip8.set_key(0xE, pressed); needs_redraw=true; break;
                    case SDLK_z: chip8.set_key(0xA, pressed); needs_redraw=true; break;
                    case SDLK_x: chip8.set_key(0x0, pressed); needs_redraw=true; break;
                    case SDLK_c: chip8.set_key(0xB, pressed); needs_redraw=true; break;
                    case SDLK_v: chip8.set_key(0xF, pressed); needs_redraw=true; break;
                    default: break;
                }
                break;
            }

            // ── Mouse (on-screen keypad) ──
            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int idx = keypad_hit(event.button.x, event.button.y);
                    if (idx >= 0) {
                        mouse_held_key = KEY_ORDER[idx];
                        chip8.set_key(mouse_held_key, true);
                        needs_redraw = true;
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (event.button.button == SDL_BUTTON_LEFT && mouse_held_key >= 0) {
                    chip8.set_key(mouse_held_key, false);
                    mouse_held_key = -1;
                    needs_redraw = true;
                }
                break;
            }
        }
    }
    return needs_redraw;
}

// ── Main ──────────────────────────────────────────────────────────────
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *rom_path = argv[1];

    Chip8 chip8;
    chip8.initialize();
    if (!chip8.loadRom(rom_path)) return EXIT_FAILURE;

    sdl_t sdl = {0};
    if (!init_SDL(&sdl)) return EXIT_FAILURE;

    bool paused       = false;
    int  cycles       = CYCLES_DEF;
    int  mouse_held   = -1;

    full_render(sdl, chip8, paused, cycles, rom_path);

    while (chip8.state != Chip8::QUIT) {
        bool redraw = handle_input(chip8, paused, cycles, rom_path, mouse_held);

        if (!paused) {
            for (int i = 0; i < cycles; ++i)
                chip8.emulateCycle();
            chip8.updateTimer();
        }

        if (chip8.draw_flag || redraw)
            full_render(sdl, chip8, paused, cycles, rom_path);

        SDL_Delay(16);
    }

    cleanup(sdl);
    return EXIT_SUCCESS;
}