#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/param.h>
#include <string.h>

#include "settings.h"
#include "iofuncs.h"
#include "gameObjects.h"
#include "assets.h"

#define QUARTERTURN 64

#define GAMEWIDTH 10
#define GAMEHEIGHT 22

#define MINOWIDTH 8
#define TEXTUREWIDTH MINOWIDTH * 4

typedef struct {int x; int y;} pair;

typedef struct {pair points[4];} tetrominoShape_t;

const tetrominoShape_t oPiece = {{
    {1, 1},
    {1, 2},
    {2, 1},
    {2, 2}
}};

const tetrominoShape_t sPiece = {{
    {1, 1},
    {0, 1},
    {1, 0},
    {2, 0}
}};

const tetrominoShape_t zPiece = {{
    {1, 1},
    {0, 0},
    {1, 0},
    {2, 1}
}};

const tetrominoShape_t tPiece = {{
    {1, 1},
    {0, 1},
    {1, 0},
    {2, 1}
}};

const tetrominoShape_t lPiece = {{
    {1, 1},
    {0, 1},
    {2, 0},
    {2, 1}
}};

const tetrominoShape_t jPiece = {{
    {1, 1},
    {0, 0},
    {0, 1},
    {2, 1}
}};

const tetrominoShape_t iPiece = {{
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1}
}};

const tetrominoShape_t* pieces[] = {
    &oPiece,
    &sPiece,
    &zPiece,
    &tPiece,
    &lPiece,
    &jPiece,
    &iPiece
};

const pixel_t tetrominoColors[] = {
    {255, 255, 0, 255},
    {0, 255, 0, 255},
    {255, 0, 0, 255},
    {128, 0, 255, 255},
    {255, 128, 0, 255},
    {0, 0, 255, 255},
    {0, 255, 255, 255}
};

typedef struct {
    int x;
    int y;
    uint8_t rotation;
    uint8_t shape;
} tetromino_t;

tetromino_t randomTetromino() {
    return (tetromino_t) {
        4,
        0,
        0,
        rand() % 7
    };
}

pixel_t imageBuffer[TEXTUREWIDTH*TEXTUREWIDTH*7*4 + (7 * sizeof(image_t))];
int globalImageId = 0;

void drawMino(pixel_t* buffer, int bufferWidth, int bufferHeight, pixel_t color, int x, int y) {
        int px = x * MINOWIDTH;
        int py = y * MINOWIDTH;
        for (int y = 0; y < MINOWIDTH; y++) {
            for (int x = 0; x < MINOWIDTH; x++) {
                pixel_t newColor = color;
                if (x == 0 || y == 0) {
                    newColor.r = MAX(((int)newColor.r) - 50, 0);
                    newColor.g = MAX(((int)newColor.g) - 50, 0);
                    newColor.b = MAX(((int)newColor.b) - 50, 0);
                }
                if (x == MINOWIDTH - 1 || y == MINOWIDTH - 1) {
                    newColor.r = MIN(((int)newColor.r) + 50, 255);
                    newColor.g = MIN(((int)newColor.g) + 50, 255);
                    newColor.b = MIN(((int)newColor.b) + 50, 255);
                }
                buffer[(y + py) * bufferWidth + x + px] = newColor;
            }
        }
}

void drawTetromino(pixel_t* buffer, tetrominoShape_t shape, int index) {
    for (int i = 0; i < 4; i++) {
        pair p = shape.points[i];
        drawMino(buffer, TEXTUREWIDTH, TEXTUREWIDTH, tetrominoColors[index], p.x, p.y);
    }
}

tetrominoShape_t getShapeRotated(int index, int rotations) {
    tetrominoShape_t shape = *(pieces[index]);
    if (rotations == 0 || index == 0) return shape;
    if (index == 6) {
        if (rotations == 2) return shape;
        return (tetrominoShape_t) {{
            {2, 0},
            {2, 1},
            {2, 2},
            {2, 3}
        }};
    }
    pair center = shape.points[0];
    bool negateX = rotations > 0 && rotations < 3;
    bool negateY = rotations > 1;
    bool swap = (rotations % 2) == 1;
    for (int i = 0; i < 4; i++) {
        pair thisPoint = shape.points[i];
        thisPoint.x -= center.x;
        thisPoint.y -= center.y;

        if (swap) {
            int temp = thisPoint.x;
            thisPoint.x = thisPoint.y;
            thisPoint.y = temp;
        }
        thisPoint.x *= 1 - (negateX * 2);
        thisPoint.y *= 1 - (negateY * 2);
        thisPoint.x += center.x;
        thisPoint.y += center.y;
        shape.points[i] = thisPoint;
    }
    return shape;
}

void makeImages(image_t* imageList[]) {
    image_t* thisImage = (image_t*)imageBuffer;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 4; j++) {
            thisImage->id = globalImageId++;
            thisImage->width = TEXTUREWIDTH;
            thisImage->height = TEXTUREWIDTH;
            thisImage->cacheAllowed = true;
            drawTetromino(thisImage->pixels, getShapeRotated(i, j), i);
            imageList[i * 4 + j] = thisImage;
            thisImage = (image_t*)((uint8_t*)thisImage + (sizeof(image_t) + TEXTUREWIDTH * TEXTUREWIDTH * sizeof(pixel_t)));
        }
    }
}

pixel_t colorAt(int x, int y, pixel_t grid[GAMEWIDTH][GAMEHEIGHT]) {
    static pixel_t black = {.r=0,.g=0,.b=0,.a=255};
    if (x < 0) return black;
    if (x >= GAMEWIDTH) return black;
    if (y < 0) return black;
    if (y >= GAMEHEIGHT) return black;
    return grid[x][y];
}

pixel_t* colorAtWrite(int x, int y, pixel_t grid[GAMEWIDTH][GAMEHEIGHT]) {
    static pixel_t black;
    black = (pixel_t) {.r=0,.g=0,.b=0,.a=255};
    if (x < 0) return &black;
    if (x >= GAMEWIDTH) return &black;
    if (y < 0) return &black;
    if (y >= GAMEHEIGHT) return &black;
    return &grid[x][y];
}

bool tryTransform(tetromino_t* original, tetromino_t* potential, pixel_t grid[GAMEWIDTH][GAMEHEIGHT]) {
    tetrominoShape_t newShape = getShapeRotated(potential->shape, potential->rotation / QUARTERTURN);
    for (int i = 0; i < 4; i++) {
        int x = newShape.points[i].x + potential->x;
        int y = newShape.points[i].y + potential->y;
        if (colorAt(x, y, grid).a == 0) {
            continue;
        };
        *potential = *original;
        return false;
    }
    return true;
}

void killTetromino(tetromino_t tetromino, pixel_t grid[GAMEWIDTH][GAMEHEIGHT]) {
    tetrominoShape_t shape = getShapeRotated(tetromino.shape, tetromino.rotation / QUARTERTURN);
    for (int i = 0; i < 4; i++) {
        pair p = shape.points[i];
        int x = tetromino.x + p.x;
        int y = tetromino.y + p.y;
        grid[x][y] = tetrominoColors[tetromino.shape];
    }
}

pixel_t backgroundBuffer[MINOWIDTH * MINOWIDTH * sizeof(pixel_t) + sizeof(image_t)];
image_t* imageHeader = (image_t*)backgroundBuffer;
spriteSheet_t gridSheet = {
    &imageHeader,
    1,
    0,
    (rect_t){.x=0, .y=0, .width=-1, .height=-1},
    MINOWIDTH,
    MINOWIDTH
};
spriteSetTrait gridSpriteSet = {
    &gridSheet,
    1,
    0
};

gameObject_t gridGameObject = {
    0,
    0,
    0,
    1
};

void initGrid() {
    imageHeader->id = globalImageId++;
    imageHeader->width = MINOWIDTH;
    imageHeader->height = MINOWIDTH;
    imageHeader->cacheAllowed = false;
}

// TODO: cache mino drawings
void drawGrid(pixel_t grid[GAMEWIDTH][GAMEHEIGHT], int xGridBase, int yGridBase) {
    pixel_t color;
    for (int y = 0; y < GAMEHEIGHT + 2; y++) {
        for (int x = 0; x < GAMEWIDTH + 2; x++) {
            if (x == 0 || x == GAMEWIDTH + 1 || y == 0 || y == GAMEHEIGHT + 1) color = (pixel_t) {255, 255, 255, 255};
            else color = colorAt(x-1, y-1, grid);
            drawMino(imageHeader->pixels, MINOWIDTH, MINOWIDTH, color, 0, 0);
            gridGameObject.x = xGridBase + (x * MINOWIDTH) - MINOWIDTH;
            gridGameObject.y = yGridBase + (y * MINOWIDTH) - MINOWIDTH;
            drawObject((interface drawable) {&gridGameObject, &gridSpriteSet});
        }
    }
}


void shiftRows(int y, pixel_t grid[GAMEWIDTH][GAMEHEIGHT]) {
    for (; y > 0; y--) {
        for (int x = 0; x < GAMEWIDTH; x++) {
            pixel_t* pix = colorAtWrite(x, y, grid);
            *pix = colorAt(x, y-1, grid);
        }
    }
}

int score = 0;

void clearRows(pixel_t grid[GAMEWIDTH][GAMEHEIGHT]) {
    for (int y = 0; y < GAMEHEIGHT; y++) {
        for (int x = 0; x < GAMEWIDTH; x++) {
            if (colorAt(x, y, grid).a == 0) goto nextRow;
        }
        printf("clearing row %d\n", y);
        shiftRows(y, grid);
        score += 1000;
nextRow:;
    }
}

void tetris_main() {
    srand(getSeed());
    inputStruct_t inputs;
    image_t* imageList[28];
    makeImages(imageList);

    inputStruct_t inputBuffers[2];
    int bufferIndex = 0;

    spriteSheet_t commonSheet = {
        imageList,
        4,
        0,
        (rect_t){.x=0, .y=0, .width=-1, .height=-1},
        TEXTUREWIDTH,
        TEXTUREWIDTH
    };
    spriteSetTrait commonSpriteSet = {
        &commonSheet,
        1,
        0
    };
    gameObject_t commonGameObject = {
        0,
        0,
        0,
        1
    };
    pixel_t grid[GAMEWIDTH][GAMEHEIGHT] = {0};
    memset(grid, 0, sizeof grid);
    initGrid();
    int xGridBase = (SCREENWIDTH / 2) - ((GAMEWIDTH + 2) * MINOWIDTH  / 2);
    int yGridBase = (SCREENHEIGHT / 2) - ((GAMEHEIGHT + 2) * MINOWIDTH / 2);
    tetromino_t live = randomTetromino();
    tetromino_t laggard = live;
    int fallTimer = 1;
    const int fallFrequency = FPS;
    while (true) {
        commonSheet.images = imageList + (live.shape * 4);
        commonSheet.images += live.rotation / QUARTERTURN;
        inputStruct_t* inputs = inputBuffers + bufferIndex;
        inputStruct_t* oldInputs = inputBuffers + !bufferIndex;
        pollInputs(inputs);
        laggard = live;
        if ((inputs->xAxis > 0) && !(oldInputs->xAxis > 0)) live.x += 1;
        else if ((inputs->xAxis < 0) && !(oldInputs->xAxis < 0)) live.x -= 1;
        tryTransform(&laggard, &live, grid);
        laggard = live;
        if (inputs->yAxis < 0) fallTimer = 0;
        tryTransform(&laggard, &live, grid);
        laggard = live;
        if (inputs->action1 && !(oldInputs->action1)) live.rotation += QUARTERTURN;
        tryTransform(&laggard, &live, grid);
        laggard = live;
        if (inputs->action2 && !(oldInputs->action2)) live.rotation -= QUARTERTURN;
        tryTransform(&laggard, &live, grid);
        laggard = live;
        if ((fallTimer++ % fallFrequency) == 0) live.y++;
        bool fallWorked = tryTransform(&laggard, &live, grid);
        if (!fallWorked) {
            killTetromino(live, grid);
            live = randomTetromino();
        }
        bufferIndex = !bufferIndex;
        *oldInputs = *inputs;
        commonGameObject.x = live.x * MINOWIDTH + xGridBase;
        commonGameObject.y = live.y * MINOWIDTH + yGridBase;
        drawObject((interface drawable) {&commonGameObject, &commonSpriteSet});
        clearRows(grid);
        drawGrid(grid, xGridBase, yGridBase);
        updateIO();
        awaitNextTick();
    }
}
