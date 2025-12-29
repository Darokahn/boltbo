#include <TFT_eSPI.h>

#include <Bonezegei_XPT2046.h>

#define ALPHAIGNORETHRESHOLD 10

#define FULLLCDWIDTH 320
#define FULLLCDHEIGHT 240

#define LCDWIDTH 265
#define LCDHEIGHT 208

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

#define BUZZER 25

#define TS_CS 5

#define VRX_PIN 33
#define VRY_PIN 32
#define SW_PIN1 26
#define SW_PIN2 27

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tftSprite = TFT_eSprite(&tft);

Bonezegei_XPT2046 ts(5, -1);

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
extern "C" void updateIO() {
    tftSprite.pushSprite(0, 0);
    tftSprite.fillSprite(TFT_BLACK);
}

extern "C" void* mallocDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

rect_t screenRect = (rect_t) {0, 0, LCDWIDTH, LCDHEIGHT};
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
    if (!rectsCollide(destination, screenRect)) return;
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

const int DEADZONE = 1000;         // Joystick deadzone threshold

extern "C" int getInput(int index) {
    int vrx = -(analogRead(VRX_PIN) - AXISMID);
    int vry = analogRead(VRY_PIN) - AXISMID;
    int sw1  = digitalRead(SW_PIN1) == LOW;  // Active-low button
    int sw2 = digitalRead(SW_PIN2) == LOW;

    if (abs(vrx) < DEADZONE) vrx = 0;
    if (abs(vry) < DEADZONE) vry = 0;

    machineLog("%d, %d\r\n", vrx, vry);

    switch (index) {
        case 0: // UP
            return vrx;
        case 1: // RIGHT
            return vry;
        case 2: // SPACE / action button
            return sw1;
        case 3:
            return sw2;
    }
    return false;
}

void pollInputs(inputStruct_t* inputs) {
    inputs->xAxis = getInput(0);
    inputs->yAxis = getInput(1);
    inputs->action1 = getInput(2);
    inputs->action2 = getInput(3);
}

int getSeed() {
    return esp_random();
}

extern "C" void startIO(int screenWidth, int screenHeight, int fps) {
    ts.begin();
    tft.init();
    tft.setRotation(1);
    tftSprite.createSprite(LCDWIDTH, LCDHEIGHT);
    tft.fillScreen(TFT_BLACK);
    nextTick = millis() + frameInterval;
    pinMode(21, OUTPUT);
    pinMode(SW_PIN1, INPUT_PULLUP);
    pinMode(SW_PIN2, INPUT_PULLUP);
    ledcAttach(BUZZER, 800, 8);
    digitalWrite(21, HIGH);
}

extern "C" void setBuzz(uint8_t value) {
    ledcWrite(BUZZER, value);
}


extern "C" int machineLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = Serial.vprintf(fmt, args);
    va_end(args);
    return n;
}

void loop() {
}

void setup() {
    Serial.begin(115200);
    main();
}
