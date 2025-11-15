#pragma once
#include <stdint.h>

extern uint8_t imageBlob[];

int loadImages(image_t** imageList, int listCapacity, uint8_t* buffer);

