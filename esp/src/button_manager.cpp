#include "button_manager.h"
#include <M5Unified.h>

static unsigned long lastInteraction = 0;
static bool displaySleeping = false;

// -------------------
bool isDisplaySleeping() {
    return displaySleeping;
}

// -------------------
void wakeDisplay() {
    displaySleeping = false;
    lastInteraction = millis();

    M5.Display.wakeup();
    M5.Display.setBrightness(120);
}

// -------------------
void initButtons() {
    lastInteraction = millis();
}

// -------------------
ButtonAction updateButtons() {
    M5.update();

    bool touched =
        M5.BtnA.wasPressed() ||
        M5.BtnB.wasPressed() ||
        M5.BtnA.isPressed() ||
        M5.BtnB.isPressed();

    // any interaction = refresh timer
    if (touched) {
        lastInteraction = millis();
    }

    // auto sleep 15 sec
    if (
        !displaySleeping &&
        millis() - lastInteraction >= 15000
    ) {
        displaySleeping = true;
        M5.Display.sleep();
        M5.Display.setBrightness(0);
    }

    // wake only (consume first press)
    if (displaySleeping && touched) {
        wakeDisplay();
        return ButtonAction::NONE;
    }

    // combo reset
    if (
        M5.BtnA.isPressed() &&
        M5.BtnB.isPressed() &&
        M5.BtnA.pressedFor(3000) &&
        M5.BtnB.pressedFor(3000)
    ) {
        return ButtonAction::RESET_WIFI;
    }

    // hold portal
    if (M5.BtnB.pressedFor(2000)) {
        return ButtonAction::START_PORTAL;
    }

    // short B
    if (M5.BtnB.wasReleased()) {
        return ButtonAction::OPEN_PORTAL_GUIDE;
    }

    // short A
    if (M5.BtnA.wasReleased()) {
        return ButtonAction::NEXT_PAGE;
    }

    return ButtonAction::NONE;
}