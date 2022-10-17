//
// Created by admin on 2022/10/17.
//

#include <SDL.h>

// 1. 使用SDL进行游戏开发系列介绍
// https://lazyfoo.net/tutorials/SDL/
// * SDL使用介绍
// * SDL的常规操作
// * 帧率和定时器
// * 移动和碰撞
// * 音视频
// * 纹理
// * 多线程
//
// 2. SDL2安装和使用
// 监听鼠标的上、下、左、右键显示不同的图片
// * SDL2安装（使用Conan）
// * 创建窗口
// * 显示图片
// * 事件监听
// * 按键事件

const int kScreenWidth = 640;
const int kScreenHeight = 480;

enum KeyPressSurfaces {
    kKeyPressSurfaceDefault,
    kKeyPressSurfaceUp,
    kKeyPressSurfaceDown,
    kKeyPressSurfaceLeft,
    kKeyPressSurfaceRight,
    kKeyPressSurfaceTotal
};

// The window we'll be rendering to.
SDL_Window* window = nullptr;

// The surface contained by the window.
SDL_Surface* screen_surface = nullptr;

// The image we will load and show on the screen
SDL_Surface* key_press_surfaces[kKeyPressSurfaceTotal];

// Current display image.
SDL_Surface* current_surface = nullptr;

// Starts up SDL and create window.
bool init() {
    // Initialization flag
    bool success = true;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    } else {
        // Create window
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  kScreenWidth,
                                  kScreenHeight,
                                  SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        } else {
            // get window surface
            screen_surface = SDL_GetWindowSurface(window);
        }
    }
    return success;
}

SDL_Surface* LoadSurface(const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (surface == nullptr)
        printf("Unable to load image %s! SDL Error: %s\n",
               path, SDL_GetError());
    return surface;
}

bool LoadMedia(const char* file_press,
               const char* file_up,
               const char* file_down,
               const char* file_left,
               const char* file_right) {
    bool success = true;

    // load default surface
    key_press_surfaces[kKeyPressSurfaceDefault] =
            LoadSurface(file_press);
    if (key_press_surfaces[kKeyPressSurfaceDefault] == nullptr) {
        printf("Failed to load default image!\n");
        success = false;
    }

    // load up surface
    key_press_surfaces[kKeyPressSurfaceUp] =
            LoadSurface(file_up);
    if (key_press_surfaces[kKeyPressSurfaceUp] == nullptr) {
        printf("Failed to load up image!\n");
        success = false;
    }

    // load down surface
    key_press_surfaces[kKeyPressSurfaceDown] =
            LoadSurface(file_down);
    if (key_press_surfaces[kKeyPressSurfaceDown] == nullptr) {
        printf("Failed to load down image!\n");
        success = false;
    }

    // load left surface
    key_press_surfaces[kKeyPressSurfaceLeft] =
            LoadSurface(file_left);
    if (key_press_surfaces[kKeyPressSurfaceLeft] == nullptr) {
        printf("Failed to load left image!\n");
        success = false;
    }

    // load right surface
    key_press_surfaces[kKeyPressSurfaceRight] =
            LoadSurface(file_right);
    if (key_press_surfaces[kKeyPressSurfaceRight] == nullptr) {
        printf("Failed to load right image!\n");
        success = false;
    }

    return success;
}

void close() {
    // Deallocate surface
    for (auto& key_press_surface: key_press_surfaces) {
        SDL_FreeSurface(key_press_surface);
        key_press_surface = nullptr;
    }

    // Destroy window
    SDL_DestroyWindow(window);
    window = nullptr;

    // Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        printf("Argument error");
        return EXIT_FAILURE;
    }
    if (!init()) {
        printf("Failed to initialize!\n");
    } else {
        if (!LoadMedia(argv[1],
                       argv[2],
                       argv[3],
                       argv[4],
                       argv[5])) {
            printf("Failed to load media!\n");
        } else {
            // main loop flag
            bool quit = false;

            // event handler
            SDL_Event e;

            // set defualt current surface
            current_surface = key_press_surfaces[
                    kKeyPressSurfaceDefault];

            // While application is running
            while (!quit) {
                // handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // user requests quit
                    if (e.type == SDL_QUIT)
                        quit = true;
                    else if (e.type == SDL_KEYDOWN) {
                        // select surface based on key press
                        switch (e.key.keysym.sym) {
                            case SDLK_UP:
                                current_surface =
                                        key_press_surfaces[kKeyPressSurfaceUp];
                                break;
                            case SDLK_DOWN:
                                current_surface =
                                        key_press_surfaces[kKeyPressSurfaceDown];
                                break;
                            case SDLK_LEFT:
                                current_surface =
                                        key_press_surfaces[kKeyPressSurfaceLeft];
                                break;
                            case SDLK_RIGHT:
                                current_surface =
                                        key_press_surfaces[kKeyPressSurfaceRight];
                                break;
                            default:
                                current_surface =
                                        key_press_surfaces[kKeyPressSurfaceDefault];
                                break;
                        }
                    }
                }

                // apple the image
                SDL_BlitSurface(current_surface, nullptr,
                                screen_surface, nullptr);

                // update the surface
                SDL_UpdateWindowSurface(window);
            }
        }
    }

    close();

    return 0;
}
