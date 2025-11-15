#pragma once

#include "settings.h"
#include "gameObjects.h"
#include "inputs.h"

void startIO();
void updateIO();

void pollInputs(uint8_t* inputBuffer);
void awaitNextTick();
