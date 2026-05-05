#pragma once
#include "types.h"

void initButtons();
ButtonAction updateButtons();

// sleep / wake
bool isDisplaySleeping();
void wakeDisplay();