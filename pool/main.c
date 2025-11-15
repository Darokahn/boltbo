#include "gameObjects.h"
#include "iofuncs.h"
#include "settings.h"
#include "assets.h"

int main() {
    startIO();
    image_t* imageList[16];
    loadImages(imageList, sizeof(imageList) / sizeof(imageList[0]), imageBlob);
    
    entity_t player = initEntity(0, 0, 20, 20, 0, 6, imageList, 1, (rect_t) {0, 0, 10, 10});
    entity_t enemies[ENEMYMAX];
    
    accelerate((interface physics) {player.object, &player.physics}, 2, 2);
    int i = 0;
    while (++i) {
        drawObject((interface drawable) {player.object, &player.spriteSheet});
        nextFrame((interface drawable) {player.object, &player.spriteSheet});
        move((interface physics) {player.object, &player.physics});
        applyAirResistance((interface physics) {player.object, &player.physics});
        updateIO();
        awaitNextTick();
    }
}
