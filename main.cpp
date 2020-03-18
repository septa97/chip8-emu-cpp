#include <cstdio>
#include <thread>
#include <SDL2/SDL.h>
#define KEYPAD_SIZE 16
#define WIDTH 64
#define HEIGHT 32

#include "chip8.cpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Keycode keymap[KEYPAD_SIZE] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v
};

void draw_frame(SDL_Texture* texture, SDL_Renderer* renderer, uint32_t* pixels) {
    SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    SDL_Event e;

    uint32_t pixels[64 * 32];
    // bool pixels[64 * 32];

    Chip8 chip8 = Chip8();

    if (argc != 2) {
        printf("Usage: ./chip8 <path-to-ROM-file>\n");
        return 1;
    }

    printf("ROM file: %s\n", argv[1]);

    chip8.initiliaze();

    if (!chip8.load_rom(argv[1])) {
        printf("Unable to load ROM file.\n");
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 3;
    }

    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32); // Use #define on this

    if (texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s", SDL_GetError());
    }
        
    bool quit = false;

    while (!quit) {
        chip8.emulate_cycle();

        while (SDL_PollEvent(&e)) { // 1 if there's an event, 0 if none
            // user requests to quit
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                for (int i = 0; i < KEYPAD_SIZE; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 1;
                    }
                }
            } else if (e.type == SDL_KEYUP) {
                for (int i = 0; i < KEYPAD_SIZE; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 0;
                    }
                }
            }
        }

        // If drawFlag is true, redraw SDL screen
        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            // Store CHIP-8 gfx to pixels
            for (int i = 0; i < 64 * 32; i++) {
                uint8_t pixel = chip8.gfx[i];
                // pixel values are either 1 or 0, multiply it by 0x00FFFFFF (which represents RGB) for either black or white
                // then bit mask with (using OR) on 0xFF000000
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;

                // here's a more verbose implementation of the above line:
                // pixels[i] = pixel ? 0xFFFFFFFF : 0xFF000000;
                // but bit ops are fancier ;)
            }

            draw_frame(texture, renderer, pixels);
        }

        // TODO: research on emulator speed
        // std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
