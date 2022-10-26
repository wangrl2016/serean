//
// Created by wangrl2016 on 2022/10/18.
//

#include <cstdlib>
#include <cstdio>
#include <string>
#include <SDL.h>
#include <SDL_image.h>

const int kScreenWidth = 1280;
const int kScreenHeight = 960;

SDL_Window* window = nullptr;
// The window renderer.
SDL_Renderer* renderer = nullptr;
// Current displayed texture.
SDL_Texture* texture = nullptr;

bool init() {
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    } else {
        // Set texture filtering to linear.
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            printf("Warning: Linear texture filtering not enabled!");
        }

        // create window
        window = SDL_CreateWindow("SDL Tutorial",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  kScreenWidth,
                                  kScreenHeight,
                                  SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            printf("Window could not be created! SDL Error: %s\n",
                   SDL_GetError());
            success = false;
        } else {
            // create renderer for window
            renderer = SDL_CreateRenderer(window, -1,
                                          SDL_RENDERER_ACCELERATED);
            if (renderer == nullptr) {
                printf("Renderer could not be created! SDL Error: %s\n",
                       SDL_GetError());
                success = false;
            } else {
                // initialize renderer color
                SDL_SetRenderDrawColor(renderer,
                                       0xFF,
                                       0xFF,
                                       0xFF,
                                       0xFF);
                // initialize PNG loading
                int img_flags = IMG_INIT_PNG;
                if (!(IMG_Init(img_flags) & img_flags)) {
                    printf("SDL_image could not initialize!"
                           "SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }
    return success;
}

SDL_Texture* LoadTexture(const std::string& path) {
    // the final texture
    SDL_Texture* new_texture = nullptr;

    // load image at specified path
    SDL_Surface* loaded_surface = IMG_Load(path.c_str());
    if (loaded_surface == nullptr) {
        printf("Unable to load image %s! SDL_image Error: %s\n",
               path.c_str(), IMG_GetError());
    } else {
        // create texture from surface pixels
        new_texture = SDL_CreateTextureFromSurface(
                renderer, loaded_surface);
        if (new_texture == nullptr) {
            printf("Unable to create texture from %s! SDL Error: %s\n",
                   path.c_str(), SDL_GetError());
        }
        // get rid of old loaded surface
        SDL_FreeSurface(loaded_surface);
    }
    return new_texture;
}

bool LoadMedia(const char* path) {
    bool success = true;
    texture = LoadTexture(path);
    if (texture == nullptr) {
        printf("Failed to load texture image!\n");
        success = false;
    }
    return success;
}

void close() {
    SDL_DestroyTexture(texture);
    texture = nullptr;

    // destroy window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = nullptr;
    renderer = nullptr;

    // quit SDL sub-systems
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Invalid argument");
        return EXIT_FAILURE;
    }
    // Start up SDL and create window
    if (!init()) {
        printf("Failed to load media!\n");
    } else {
        if (!LoadMedia(argv[1])) {
            printf("Failed to load media!\n");
        } else {
            bool quit = false;

            // Event handler
            SDL_Event e;

            // While application is running
            while (!quit) {
                // Handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // User requests quit
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }
                }

                // clear screen
                SDL_SetRenderDrawColor(renderer,
                                       0xFF,
                                       0xFF,
                                       0xFF,
                                       0xFF);
                SDL_RenderClear(renderer);

                // render texture to screen
                SDL_RenderCopy(renderer,
                               texture,
                               nullptr,
                               nullptr);

                // render red filled quad
                SDL_Rect fill_rect = {
                        kScreenWidth / 4,
                        kScreenHeight / 4,
                        kScreenWidth / 2,
                        kScreenHeight / 2};
                SDL_SetRenderDrawColor(renderer,
                                       0xFF,
                                       0x00,
                                       0x00,
                                       0xFF);
                SDL_RenderFillRect(renderer,
                                   &fill_rect);

                // render green outlined quad
                SDL_Rect outline_rect = {
                        kScreenWidth / 6,
                        kScreenHeight / 6,
                        kScreenWidth * 2 / 3,
                        kScreenHeight * 2 / 3
                };
                SDL_SetRenderDrawColor(renderer,
                                       0x00,
                                       0xFF,
                                       0x00,
                                       0xFF);
                SDL_RenderDrawRect(renderer,
                                   &outline_rect);

                // Draw blue horizontal line
                SDL_SetRenderDrawColor(renderer,
                                       0x00,
                                       0x00,
                                       0xFF,
                                       0xFF);
                SDL_RenderDrawLine(renderer,
                                   0,
                                   kScreenHeight / 2,
                                   kScreenWidth,
                                   kScreenHeight / 2);

                // draw vertical line of yellow dots
                SDL_SetRenderDrawColor(renderer,
                                       0xFF,
                                       0xFF,
                                       0x00,
                                       0xFF);
                for (int i = 0; i < kScreenHeight; i += 4) {
                    SDL_RenderDrawPoint(renderer,
                                        kScreenWidth / 2,
                                        i);
                }

                // update screen
                SDL_RenderPresent(renderer);
            }
        }
    }
    close();

    return EXIT_SUCCESS;
}
