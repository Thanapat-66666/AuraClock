#pragma once
#include <Arduino.h>
#include <vector>

void initDisplay();

void setBrightnessByTime();

// ---------- common ----------
void drawHeader(
    const String& leftText,
    const String& wifiDots
);

// ---------- routine ----------
void drawRoutinePage(
    const String& headerLeft,
    const String& wifiDots,
    const std::vector<String>& steps,
    int countdown
);

// ---------- clock ----------
void drawClockPage(
    const String& wifiDots,
    const String& dateText,
    const String& timeText
);

void drawInfoPage(
    const String& wifiDots,
    int batteryPercent,
    bool charging,
    bool lowBattery
);

void drawPortalGuidePage(
    const String& wifiDots
);

void drawPortalActivePage(
    const String& wifiDots
);

// ---------- marquee ----------
String fitText(
    const String& text,
    size_t limit
);