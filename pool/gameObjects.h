#pragma once

#include "settings.h"
#include <stdbool.h>
#include <stdint.h>

#define interface struct

typedef struct {
    int x;
    int y;
} point;

typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} rect_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} pixel_t;

typedef struct {
    int16_t id;
    int16_t width;
    int16_t height;
    char name[NAMESIZE];
    pixel_t pixels[];
} image_t;

typedef struct {
    float x;
    float y;
    uint16_t id;
    bool alive;
    uint8_t traitData[];
} gameObject_t;

typedef struct {
    image_t** images;
    uint16_t imageCount;
    uint16_t currentImage;
    uint16_t xScale;
    uint16_t yScale;
} spriteTrait;

typedef struct {
    float xVelocity;
    float yVelocity;
    bool respectsGravity;
    float gravityOverride; // 0 if uses global gravity, some other number to define a specific gravity value.
    bool respectsAirResistance;
    float airResistanceOverride; // 0 if uses global air resistance, some other number to define a specific gravity value.
    float inertia;
} physicsTrait;

typedef struct {
    // TODO
} audioTrait;

struct collisionTraitGeneric {
    uint16_t collisionType;
    void* collisionData; // checks out space in its gameObject
};
typedef struct collisionTraitGeneric collisionTraitGeneric;

enum collisionType {
    RECT,
    CIRCLE,
};

typedef struct {
    uint16_t collisionType;
    rect_t* boundingBox;
} collisionTraitRect;


typedef union {
    collisionTraitGeneric generic;
    collisionTraitRect rect;
} collisionTrait;

// Basic interfaces that primitive functions use. "top-level interfaces", interface structs that do not compose any larger types, should be used as each object type.
// top-level interfaces should have init methods, and non top-level interfaces should be derived from those.
// Objects that are not the same top-level interface but can interact via sub-interfaces (all physics objects, for example) can derive and distribute interfaces into polymorphic lists.

interface drawable {
    gameObject_t* object;
    spriteTrait* sprites;
};

void drawObject(interface drawable t);
void nextFrame(interface drawable t);

interface physics {
    gameObject_t* object;
    physicsTrait* physics;
};

void applyGravity(interface physics t);
void accelerate(interface physics t, float x, float y);
void applyAirResistance(interface physics t);
void move(interface physics t);
void applyGravity(interface physics t);

interface audio {
    gameObject_t* object;
    audioTrait* audio;
};

interface collision {
    gameObject_t* object;
    collisionTrait* collision;
};

void getCollisions(interface collision t, interface collision** others);
void collidesWith(interface collision t, interface collision* other);

interface physicsCollision {
    gameObject_t* object;
    collisionTrait* collision;
    physicsTrait* physics;
};

void moveWithCollisions(interface physicsCollision t, interface collision* others);

typedef struct {
    gameObject_t* object;
    collisionTrait collision;
    physicsTrait physics;
    spriteTrait spriteSheet;
    audioTrait audio;
} entity_t;

entity_t initEntity(float x, float y, int width, int height, int spriteSheetIndex, int spriteCount, image_t** imageList, float inertia, rect_t boundingBox);
