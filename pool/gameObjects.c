#include "gameObjects.h"
#include <stdlib.h>
#include <stdio.h>

#define align(num, size) (((intptr_t)num) + ((size) - 1) & (~((size) - 1)))
#define alignptr(num, size) (void*)(((intptr_t)num) + ((size) - 1) & (~((size) - 1)))

void nextFrame(interface drawable this) {
    spriteSheet_t* sheet = &this.sprites->spriteSheets[this.sprites->currentSheet];
    sheet->currentImage++;
    sheet->currentImage %= sheet->imageCount;
}

void setSheet(interface drawable t, int sheetIndex) {
    t.sprites->currentSheet = sheetIndex;
}

image_t* getFrame(interface drawable this) {
    spriteSheet_t* sheet = &this.sprites->spriteSheets[this.sprites->currentSheet];
    return sheet->images[sheet->currentImage];
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

bool rectsCollide(rect_t r1, rect_t r2) {
    bool collides = false;
    for (int i = 0; i < 4; i++) {
        int x = r1.x;
        int y = r1.y;
        if (i & 1) x += r1.width;
        if (i & 2) y += r1.height;
        collides |= (x >= r2.x && x <= r2.x + r2.width) && (y >= r2.y && y <= r2.y + r2.height);
    }
    return collides;
}


void getCollisions(interface collision this, interface collision* others, int numOthers) {
}

bool collidesWith(interface collision this, interface collision other) {
    collisionTraitGeneric c1 = this.collision->generic;
    collisionTraitGeneric c2 = other.collision->generic;
    switch (c1.collisionType) {
        case RECT:
            rect_t rect1 = *this.collision->rect.boundingBox;
            rect_t rect2 = *other.collision->rect.boundingBox;
            rect1.x = this.object->x;
            rect1.y = this.object->y;
            rect2.x = other.object->x;
            rect2.y = other.object->y;

            return rectsCollide(rect1, rect2);
            break;
        case CIRCLE:
            return false;
            break;
        default:
            return false;
            break;
    }
}

void moveWithCollisions(interface physicsCollision this, interface physicsCollision* others, int numOthers) {
    float oldX = this.object->x;
    float oldY = this.object->y;
    move((interface physics) {this.object, this.physics});
    applyAirResistance((interface physics) {this.object, this.physics});
    for (int i = 0; i < numOthers; i++) {
        interface physicsCollision other = others[i];
        if (other.object->id == this.object->id) continue;
        printf("%d\n", other.object->id);
        bool collides = collidesWith((interface collision) {this.object, this.collision}, (interface collision) {other.object, other.collision});
        if (collides) {
            this.object->x = oldX;
            this.object->y = oldY;
        }
    }
}

entity_t initEntity(float x, float y, int width, int height, int* spriteSheetIndices, int* frameCounts, int sheetCount, image_t** imageList, float inertia, rect_t boundingBox) {
    static int id = 0;
    entity_t entity;
    int size = sizeof(gameObject_t);
    size = align(size, __alignof(rect_t));
    size += sizeof(rect_t);
    size = align(size, __alignof(spriteSheet_t));
    size += sizeof(spriteSheet_t) * sheetCount;
    entity.object = malloc(size);
    *entity.object = (gameObject_t) {
        .x = x,
        .y = y,
        .id = id++,
    };
    entity.physics = (physicsTrait) {
        .xVelocity = 0,
        .yVelocity = 0,
        .respectsGravity = true,
        .inertia = inertia,
        .gravityOverride = 0,
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
    ptr += sizeof(rect_t);
    ptr = alignptr(ptr, __alignof(spriteSheet_t));
    entity.spriteSet = (spriteSetTrait) {
        .spriteSheets = (spriteSheet_t*) ptr,
        .sheetCount = sheetCount,
        .currentSheet = 0
    };
    for (int i = 0; i < sheetCount; i++) {
        int sheetIndex = spriteSheetIndices[i];
        int frameCount = frameCounts[i];
        entity.spriteSet.spriteSheets[i] = (spriteSheet_t) {
            .images = imageList + sheetIndex,
            .imageCount = frameCount,
            .xScale = 1,
            .yScale = 1,
            .width = width,
            .height = height,
            .ticksPerFrame = 1
        };
    }
    *entity.collision.rect.boundingBox = boundingBox;
    return entity;
}

terrain_t initTerrain(float x, float y, int width, int height, rect_t boundingBox) {
    static int id = 0;
    terrain_t terrain;
    int size = sizeof(gameObject_t);
    size = align(size, __alignof(rect_t));
    size += sizeof(rect_t);
    terrain.object = malloc(size);
    *terrain.object = (gameObject_t) {
        .x = x,
        .y = y,
        .id = id++,
    };
    terrain.physics = (physicsTrait) {
        .xVelocity = 0,
        .yVelocity = 0,
        .respectsGravity = true,
        .gravityOverride = 0,
        .respectsAirResistance = true,
        .airResistanceOverride = 0,
        .inertia = 0,
    };
    uint8_t* ptr = (void*) terrain.object;
    ptr += sizeof(gameObject_t);
    ptr = alignptr(ptr, __alignof(rect_t));
    terrain.collision = (collisionTrait) {.rect = 
        {
            .collisionType=RECT,
            .boundingBox=(rect_t*)ptr
        }
    };
    *terrain.collision.rect.boundingBox = boundingBox;
    return terrain;
}
