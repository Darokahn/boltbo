#include "sdl_wrapper.h"
#include "gameObjects.h"

#include <SDL2/SDL.h>
#include <math.h>

struct screenPackage initVideo(int width, int height, int initialTextureCapacity, int windowScale) {
    struct {int width; int height;} screenSize = {width, height};
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        perror( "SDL failed" );
        exit(1);
    }
    struct screenPackage screen;
    screen.width = screenSize.width;
    screen.height = screenSize.height;

    screen.window = SDL_CreateWindow( "Video", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenSize.width * windowScale, screenSize.height * windowScale, SDL_WINDOW_SHOWN );
    if ( screen.window == NULL ) {
        printf("%s\n", SDL_GetError());
    }
    screen.renderer = SDL_CreateRenderer(screen.window, -1, SDL_RENDERER_ACCELERATED);
    if (screen.renderer == 0) {
        printf("%s\n", SDL_GetError());
    }
    SDL_RenderSetLogicalSize(screen.renderer, width, height);
    screen.initialized = true;
    screen.textureCapacity = initialTextureCapacity;
    screen.textures = (malloc(sizeof(*screen.textures) * screen.textureCapacity));
    for (int i = 0; i < screen.textureCapacity; i++) {
        screen.textures[i] = NULL;
    }
    return screen;
}

void destroyVideo(struct screenPackage screen) {
    SDL_DestroyRenderer(screen.renderer);
    SDL_DestroyWindow(screen.window);
    SDL_Quit();
}

void updateScreen(struct screenPackage screen) {
    SDL_RenderPresent(screen.renderer);
}

void clearScreen(struct screenPackage screen, pixel_t c) {
    SDL_SetRenderDrawColor(screen.renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(screen.renderer);
}

void setPixel(struct screenPackage screen, point_t p, pixel_t c) {
    SDL_SetRenderDrawColor(screen.renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(screen.renderer, p.x, p.y);
}
