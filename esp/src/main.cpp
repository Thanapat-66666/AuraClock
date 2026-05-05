#include <Arduino.h>
#include <vector>

#include "display_manager.h"
#include "button_manager.h"
#include "routine_manager.h"
#include "types.h"
#include "wifi_manager.h"
#include "storage_manager.h"

ClockPage currentPage = ClockPage::ROUTINE;

String lastHeader = "";
String lastWifi = "";
String lastTime = "";
String lastDate = "";
int lastBattery = -1;
int lastCountdown = -1;
String lastRoutineKey = "";

// -------------------
void forceRefresh() {
    lastHeader = "";
    lastWifi = "";
    lastTime = "";
    lastDate = "";
    lastBattery = -1;
    lastCountdown = -1;
    lastRoutineKey = "";
}

// -------------------
void nextPage() {
    switch (currentPage) {
        case ClockPage::ROUTINE:
            currentPage = ClockPage::CLOCK;
            break;

        case ClockPage::CLOCK:
            currentPage = ClockPage::INFO;
            break;

        case ClockPage::INFO:
            currentPage = ClockPage::ROUTINE;
            break;

        default:
            currentPage = ClockPage::ROUTINE;
            break;
    }

    forceRefresh();
}

// -------------------
void setup() {
    initDisplay();
    initButtons();
    initWiFi();
}

// -------------------
void loop() {
    ButtonAction action = updateButtons();

    handlePortal();

    // sleeping -> background only
    if (isDisplaySleeping()) {
        delay(30);
        return;
    }

    // actions
    if (action == ButtonAction::NEXT_PAGE) {
        nextPage();
    }
    else if (action == ButtonAction::OPEN_PORTAL_GUIDE) {
        currentPage = ClockPage::PORTAL_GUIDE;
        forceRefresh();
    }
    else if (action == ButtonAction::START_PORTAL) {
        startPortal();
        currentPage = ClockPage::PORTAL_ACTIVE;
        forceRefresh();
    }
    else if (action == ButtonAction::RESET_WIFI) {
        clearWiFi();
        startPortal();

        currentPage = ClockPage::PORTAL_ACTIVE;
        forceRefresh();
    }

    // brightness only when awake
    setBrightnessByTime();

    String header = getDayTodayText();
    String wifi = getWifiDots();
    String timeText = getTimeString();
    String dateText = getThaiDateString();

    int battery = getBatteryPercent();
    bool charging = isCharging();
    bool lowBattery = isLowBattery();

    int countdown = getCountdown();

    // ---------------- ROUTINE
    if (currentPage == ClockPage::ROUTINE) {
        auto steps = getRoutineSteps();

        String key = "";
        for (auto& s : steps) key += s;

        if (
            key != lastRoutineKey ||
            countdown != lastCountdown ||
            header != lastHeader ||
            wifi != lastWifi
        ) {
            drawRoutinePage(
                header,
                wifi,
                steps,
                countdown
            );

            lastRoutineKey = key;
            lastCountdown = countdown;
            lastHeader = header;
            lastWifi = wifi;
        }
    }

    // ---------------- CLOCK
    else if (currentPage == ClockPage::CLOCK) {
        if (
            timeText != lastTime ||
            dateText != lastDate ||
            wifi != lastWifi
        ) {
            drawClockPage(
                wifi,
                dateText,
                timeText
            );

            lastTime = timeText;
            lastDate = dateText;
            lastWifi = wifi;
        }
    }

    // ---------------- INFO
    else if (currentPage == ClockPage::INFO) {
        if (
            battery != lastBattery ||
            wifi != lastWifi
        ) {
            drawInfoPage(
                wifi,
                battery,
                charging,
                lowBattery
            );

            lastBattery = battery;
            lastWifi = wifi;
        }
    }

    // ---------------- PORTAL
    else if (currentPage == ClockPage::PORTAL_GUIDE) {
        drawPortalGuidePage(wifi);
        delay(80);
    }
    else if (currentPage == ClockPage::PORTAL_ACTIVE) {
        drawPortalActivePage(wifi);
        delay(80);
    }

    delay(30);
}