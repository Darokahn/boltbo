#include <stdbool.h>

#include "settings.h"
#include "gameObjects.h"
#include "sdl_wrapper.h"
#include "iofuncs.h"

pixel_t black = {.r=0, .g=0, .b=0};
struct screenPackage s;
SDL_Event e;

uint32_t nextTick;

void startIO() {
    s = initVideo(SCREENWIDTH, SCREENHEIGHT, 64, 4);
    nextTick = SDL_GetTicks() + (1000 / FPS);
}

void updateIO() {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            exit(0);
        }
    }
    updateScreen(s);
    clearScreen(s, black);
}

SDL_Texture* getTexture(image_t* image) {
    SDL_Texture* texture;
    SDL_Surface* surface;
    if (image->id > s.textureCapacity) {
        s.textureCapacity *= 2;
        s.textures = realloc(s.textures, s.textureCapacity * sizeof (*s.textures));
    }
    if (s.textures[image->id] == NULL) {
        surface = SDL_CreateRGBSurfaceWithFormatFrom(
                image->pixels,
                image->width,
                image->height,
                32,
                image->width * 4,
                SDL_PIXELFORMAT_ABGR8888
                );
        if (surface == NULL) {
            fprintf(stderr, "getTexture: %s\n", SDL_GetError());
            return NULL;
        }
        s.textures[image->id] = SDL_CreateTextureFromSurface(s.renderer, surface);
        SDL_FreeSurface(surface);
    }
    if (s.textures[image->id] == NULL) {
        fprintf(stderr, "getTexture: %s\n", SDL_GetError());
        return NULL;
    }
    return s.textures[image->id];
}

void drawObject(interface drawable drawable) {
    gameObject_t* object = drawable.object;
    spriteSetTrait* spriteSet = drawable.sprites;
    spriteSheet_t* spriteSheet = &spriteSet->spriteSheets[spriteSet->currentSheet];
    image_t* image = getFrame(drawable);
    SDL_Texture* texture = getTexture(image);
    SDL_Rect destination = (SDL_Rect) {
        .x = (int)object->x,
        .y = (int)object->y,
        .w = spriteSheet->width * spriteSheet->xScale,
        .h = spriteSheet->height * spriteSheet->yScale,
    };
    SDL_RenderCopy(s.renderer, texture, NULL, &destination);
}

void awaitNextTick() {
    while (SDL_GetTicks() < nextTick);
    nextTick += 1000 / FPS;
}
