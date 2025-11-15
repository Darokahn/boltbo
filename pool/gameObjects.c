#include "gameObjects.h"
#include <stdlib.h>

#define align(num, size) (((intptr_t)num) + ((size) - 1) & (~((size) - 1)))
#define alignptr(num, size) (void*)(((intptr_t)num) + ((size) - 1) & (~((size) - 1)))

void nextFrame(interface drawable this) {
    this.sprites->currentImage++;
    this.sprites->currentImage %= this.sprites->imageCount;
}

void applyGravity(interface physics this) {
    int gravity = GRAVITY;
    if (this.physics->gravityOverride) gravity = this.physics->gravityOverride;
    accelerate(this, 0, gravity);
}
void accelerate(interface physics this, float x, float y) {
    float inertia = this.physics->inertia;
    x /= inertia;
    y /= inertia;
    this.physics->xVelocity += x;
    this.physics->yVelocity += y;
}
void move(interface physics this) {
    this.object->x += this.physics->xVelocity;
    this.object->y += this.physics->yVelocity;
}

void applyAirResistance(interface physics this) {
    float airResistance = AIRRESISTANCE;
    if (this.physics->airResistanceOverride) airResistance = this.physics->airResistanceOverride;
    this.physics->xVelocity *= airResistance;
    this.physics->yVelocity *= airResistance;
}

void getCollisions(interface collision this, interface collision** others) {}
void collidesWith(interface collision this, interface collision* other) {}

void moveWithCollisions(interface physicsCollision this, interface collision* others) {}

entity_t initEntity(float x, float y, int width, int height, int spriteSheetIndex, int spriteCount, image_t** imageList, float inertia, rect_t boundingBox) {
    static int id = 0;
    entity_t entity;
    int size = sizeof(gameObject_t);
    size = align(size, __alignof(rect_t));
    size += sizeof(rect_t);
    entity.object = malloc(size);
    *entity.object = (gameObject_t) {
        .x = x,
        .y = y,
        .id = id++,
    };
    uint8_t* ptr = (void*) entity.object;
    ptr += sizeof(gameObject_t);
    ptr = alignptr(ptr, __alignof(rect_t));
    entity.collision = (collisionTrait) {.rect = 
        {
            .collisionType=RECT,
            .boundingBox=(rect_t*)ptr
        }
    };
    entity.spriteSheet = (spriteTrait) {
        .images = imageList + spriteSheetIndex,
        .imageCount = spriteCount,
        .currentImage = 0,
        .xScale = 1,
        .yScale = 1
    };
    *entity.collision.rect.boundingBox = boundingBox;
    entity.physics = (physicsTrait) {
        .xVelocity = 0,
        .yVelocity = 0,
        .respectsGravity = true,
        .inertia = inertia,
        .gravityOverride = 0,
    };
    return entity;
}
