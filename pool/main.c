#include <stdio.h>

#include "gameObjects.h"
#include "iofuncs.h"
#include "settings.h"
#include "assets.h"

extern int pointCount;

int main() {
    startIO();
    image_t* imageList[16];
    loadImages(imageList, sizeof(imageList) / sizeof(imageList[0]), imageBlob);
    
    entity_t background = initEntity(0, 0, 320, 240,  (int[1]) {0}, (int[1]) {1}, 1, imageList, 0, (rect_t) {});
    entity_t player = initEntity(0, 0, 48, 48,  (int[2]) {3, 4}, (int[2]) {1, 5}, 2, imageList, 1, (rect_t) {48, 48});
    setSheet(toDrawable(player), 1);
    entity_t enemies[ENEMYMAX];
    int enemyCount = 0;

    entity_t tiles[TILEMAX];
    int tileCount = 0;

    loadStage(points, pointCount, tiles, TILEMAX, imageList);
    tileCount = TILEMAX;

    interface drawable drawnEntities[DRAWINGMAX];
    int drawnCount = 0;

    interface physicsCollision physicsEntities[PHYSICSMAX];
    int physicsCount = 0;

    for (int i = 0; i < tileCount; i++) {
        physicsEntities[i] = toPhysicsCollision(tiles[i]);
        drawnEntities[i] = toDrawable(tiles[i]);
        drawnCount++;
        physicsCount++;
    }
    drawnEntities[0] = toDrawable(player);
    physicsEntities[0] = toPhysicsCollision(player);
    
    accelerate(toPhysics(player), 1, 1);
    int i = 0;
    while (++i) {
        drawObject((interface drawable) {background.object, &background.spriteSet});
        for (int i = 0; i < drawnCount; i++) {
            drawObject(drawnEntities[i]);
            nextFrame(drawnEntities[i]);
        }
        for (int i = 0; i < physicsCount; i++) {
            moveWithCollisions(physicsEntities[i], physicsEntities, physicsCount);
        }
        updateIO();
        awaitNextTick();
    }
}
