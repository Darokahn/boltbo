#include <stdio.h>
#include <sys/param.h>

#include "gameObjects.h"
#include "iofuncs.h"
#include "settings.h"
#include "assets.h"

void tetris_main();

int main() {
    startIO(320, 240, 30);
    tetris_main();
}
