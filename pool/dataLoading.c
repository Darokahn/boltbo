#include <stdint.h>
#include <stdbool.h>
#include "gameObjects.h"
#include "settings.h"
#include "assets.h"

int loadImages(image_t** imageList, int listCapacity, uint8_t* buffer) {
    image_t* image;
    int imageSize;
    for(int imageCount = 0; imageCount < listCapacity; (imageCount++, buffer += imageSize)) {
        image = (void*) buffer;
        if (image->id == -1) {
            return imageCount;
        }
        imageSize = image->width * image->height * sizeof(image->pixels[0]) + sizeof(image_t);
        imageList[imageCount] = image;
    }
    return -1;
}
