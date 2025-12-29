#include <stdbool.h>
#include <time.h>

#include "settings.h"
#include "gameObjects.h"
#include "sdl_wrapper.h"
#include "iofuncs.h"

pixel_t black = {.r=0, .g=0, .b=0};
struct screenPackage s;

uint32_t nextTick;

void startIO(int screenWidth, int screenHeight, int fps) {
    s = initVideo(SCREENWIDTH, SCREENHEIGHT, 64, 3);
    nextTick = SDL_GetTicks() + (1000 / FPS);
}

void updateIO() {
    updateScreen(s);
    clearScreen(s, black);
}

SDL_Texture* getTexture(image_t* image) {
    SDL_Texture* texture;
    SDL_Surface* surface;
    while (image->id > s.textureCapacity) {
        s.textureCapacity *= 2;
        s.textures = realloc(s.textures, s.textureCapacity * sizeof (*s.textures));
    }
    if (s.textures[image->id] == NULL || image->cacheAllowed == false) {
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
        if (s.textures[image->id] != NULL) SDL_DestroyTexture(s.textures[image->id]);
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
        .w = spriteSheet->width,
        .h = spriteSheet->height,
    };
    SDL_RenderCopy(s.renderer, texture, NULL, &destination);
}

void awaitNextTick() {
    while (SDL_GetTicks() < nextTick);
    nextTick += 1000 / FPS;
}

void pollInputs(inputStruct_t* input) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            exit(0);
        }
        else if (e.type == SDL_KEYDOWN) {
            SDL_Keycode k = e.key.keysym.sym;
            bool repeat = e.key.repeat;
            if (k == SDLK_RIGHT && !repeat) input->xAxis += AXISMID;
            else if (k == SDLK_LEFT && !repeat) input->xAxis -= AXISMID;

            else if (k == SDLK_UP && !repeat) input->yAxis += AXISMID;
            else if (k == SDLK_DOWN && !repeat) input->yAxis -= AXISMID;

            else if (k == SDLK_z) input->action2 = true;
            else if (k == SDLK_x) input->action1 = true;
        }
        else if (e.type == SDL_KEYUP) {
            SDL_Keycode k = e.key.keysym.sym;
            if (k == SDLK_RIGHT) input->xAxis -= AXISMID;
            else if (k == SDLK_LEFT) input->xAxis += AXISMID;

            if (k == SDLK_UP) input->yAxis -= AXISMID;
            else if (k == SDLK_DOWN) input->yAxis += AXISMID;

            if (k == SDLK_z) input->action2 = false;
            else if (k == SDLK_x) input->action1 = false;
        }
    }
}

int getSeed() {
    return time(NULL);
}
