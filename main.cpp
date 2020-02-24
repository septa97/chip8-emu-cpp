#include <cstdio>
#include <SDL2/SDL.h>
#define KEYPAD_SIZE 16

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

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Surface *screenSurface = NULL;
    SDL_Event e;

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
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        window = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        } else {
            screenSurface = SDL_GetWindowSurface(window);

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

                // SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

                SDL_UpdateWindowSurface(window);
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
