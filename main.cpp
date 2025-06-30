#include "chip8.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <iostream>

const int VIDEO_WIDTH = 64;
const int VIDEO_HEIGHT = 32;
const int PIXEL_SCALE = 10;
const int WINDOW_WIDTH = VIDEO_WIDTH * PIXEL_SCALE;
const int WINDOW_HEIGHT = VIDEO_HEIGHT * PIXEL_SCALE;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: chip8.exe <ROM File>\n");
        return 1;
    }

    chip8 chip;
    chip.init();
    chip.loadRom(argv[1]);
    std::cout << "ROM loaded: " << argv[1] << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("CHIP-8 Emulator", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        printf("Window Creation Failed %s \n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        printf("Renderer creation failed: %s \n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                             VIDEO_WIDTH, VIDEO_HEIGHT);

    if (!texture)
    {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Keycode keymap[16] = {
        SDLK_X, SDLK_1, SDLK_2, SDLK_3,
        SDLK_Q, SDLK_W, SDLK_E, SDLK_A,
        SDLK_S, SDLK_D, SDLK_Z, SDLK_C,
        SDLK_4, SDLK_R, SDLK_F, SDLK_V};

    bool running = true;
    SDL_Event event;
    uint32_t pixels[VIDEO_WIDTH * VIDEO_HEIGHT];

    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    while (running)
    {
        uint32_t frameStart = SDL_GetTicks();

        // SDL stuff
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            {
                bool pressed = (event.type == SDL_EVENT_KEY_DOWN);
                SDL_KeyboardEvent *keyEvent = &event.key;
                for (int i = 0; i < 16; ++i)
                {
                    if (keyEvent->key == keymap[i])
                    {
                        chip.key[i] = pressed;
                    }
                }
                break;
            }

            default:
                break;
            }
        }

        // Run multiple instructions per frame
        for (int i = 0; i < 10; ++i)
        {
            chip.emulateCycle();
        }

        // Timers
        if (chip.delay_timer > 0)
            --chip.delay_timer;

        if (chip.sound_timer > 0)
        {
            if (chip.sound_timer == 1)
                std::cout << "BEEP!\n";
            --chip.sound_timer;
        }

        // Draw graphics
        if (chip.drawFlag)
        {
            chip.drawFlag = false;
            for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; ++i)
            {
                pixels[i] = chip.graphics[i] ? 0xFFFFFFFF : 0xFF000000;
            }

            SDL_UpdateTexture(texture, nullptr, pixels, VIDEO_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);

            SDL_FRect destRect = {0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
            SDL_RenderTexture(renderer, texture, nullptr, &destRect);

            SDL_RenderPresent(renderer);
        }

        int frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
