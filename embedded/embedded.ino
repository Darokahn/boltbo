#include <TFT_eSPI.h>

#define ALPHAIGNORETHRESHOLD 10

#define FULLLCDWIDTH 320
#define FULLLCDHEIGHT 240

#define LCDWIDTH 265
#define LCDHEIGHT 208

#define SCRATCHWIDTH 480
#define SCRATCHHEIGHT 360

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tftSprite = TFT_eSprite(&tft);

extern "C" {
    #include "gameObjects.h"
    #include "iofuncs.h"
    #include "settings.h"
}

#define DRAWRATE FPS / 10
int frameInterval = 1000 / FPS;

int nextTick;
void awaitNextTick() {
    while (millis() < nextTick);
    nextTick += frameInterval;
}

extern "C" int main();
extern "C" void startIO() {
    tft.init();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_BLACK);
    nextTick = millis() + frameInterval;
}

extern "C" void updateIO() {
    tftSprite.pushSprite((FULLLCDWIDTH - LCDWIDTH) / 2, (FULLLCDHEIGHT - LCDHEIGHT) / 2);
    tftSprite.fillSprite(TFT_BLACK);
}

extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

extern "C" void drawObject(interface drawable t) {
    gameObject_t* object = t.object;
    spriteSetTrait* spriteSet = t.sprites;
    spriteSheet_t* spriteSheet = &spriteSet->spriteSheets[spriteSet->currentSheet];
    image_t* image = spriteSheet->images[spriteSheet->currentImage];
    rect_t destination = (rect_t) {
        .x = (int)object->x,
        .y = (int)object->y,
        .width = spriteSheet->width,
        .height = spriteSheet->height,
    };
    float xStride = (float)image->width / destination.width;
    float yStride = (float)image->height / destination.height;
    int pixelSize = sizeof(pixel_t);
    int rowSize = image->width * pixelSize;
    uint8_t* bytes = (uint8_t*) image->pixels;
    for (int y = 0; y < destination.height; y++) {
        for (int x = 0; x < destination.width; x++) {
            int column = x * xStride;
            int row = y * yStride;
            bytes = (uint8_t*)image->pixels + (row * rowSize) + (column * pixelSize);
            uint8_t red = *(bytes++) * 31 / 255;
            uint8_t green = *(bytes++) * 63 / 255;
            uint8_t blue = *(bytes++) * 31 / 255;
            uint8_t alpha = *(bytes++);
            if (alpha <= ALPHAIGNORETHRESHOLD) continue;
            uint16_t color = red << 11 | green << 5 | blue;
            int drawX = x + destination.x;
            int drawY = y + destination.y;
            tftSprite.drawPixel(drawX, drawY, color);
        }
    }
}

#define VRX_PIN 32
#define VRY_PIN 33
#define SW_PIN 25

const int ADC_CENTER = 1880;       // Midpoint of 12-bit ADC
const int DEADZONE  = 200;         // Joystick deadzone threshold

extern "C" bool getInput(int index) {
    int vrx = analogRead(VRX_PIN);
    int vry = analogRead(VRY_PIN);
    bool sw  = digitalRead(SW_PIN) == LOW;  // Active-low button

    //machineLog("%d, %d\r\n", vrx, vry);

    switch (index) {
        case 0: // UP
            return vry < ADC_CENTER - DEADZONE;
        case 1: // RIGHT
            return vrx > ADC_CENTER + DEADZONE;
        case 2: // DOWN
            return vry > ADC_CENTER + DEADZONE;
        case 3: // LEFT
            return vrx < ADC_CENTER - DEADZONE;
        case 4: // SPACE / action button
            return sw;
    }
    return false;
}

extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}

#include <Bonezegei_XPT2046.h>

#define TS_CS 33
#define TS_IRQ 32
Bonezegei_XPT2046 ts(TS_CS, TS_IRQ);

void loop() {
}

void setup() {
    Serial.begin(115200);
    ts.begin();
    main();
}
