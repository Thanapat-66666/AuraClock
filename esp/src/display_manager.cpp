#include "display_manager.h"
#include "wifi_manager.h"
#include <M5Unified.h>

static const uint16_t BG = 0x0841;

// -----------------------------
void drawLightning(
    int x,
    int y,
    int scale,
    uint16_t color
) {
    auto& d = M5.Display;

    d.fillTriangle(
        x + 8 * scale, y,
        x + 2 * scale, y + 16 * scale,
        x + 10 * scale, y + 16 * scale,
        color
    );

    d.fillTriangle(
        x + 10 * scale, y + 16 * scale,
        x + 4 * scale, y + 34 * scale,
        x + 16 * scale, y + 14 * scale,
        color
    );
}

// -----------------------------
String fitText(const String& text, size_t limit) {
    if (text.length() <= limit) return text;
    return text.substring(0, limit - 3) + "...";
}

// -----------------------------
void initDisplay() {
    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(0);
    M5.Display.setBrightness(120);
    M5.Display.fillScreen(BG);
    M5.Display.setTextColor(WHITE, BG);

    // IMPORTANT:
    // ไม่ set global font
    M5.Display.setFont(nullptr);
}

// -----------------------------
void setBrightnessByTime() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    int h = t->tm_hour;

    int brightness = 120;

    if (h >= 6 && h < 18) brightness = 120;
    else if (h >= 18 && h < 23) brightness = 80;
    else brightness = 25;

    M5.Display.setBrightness(brightness);
}

// -----------------------------
void drawHeader(
    const String& leftText,
    const String& wifiDots
) {
    auto& d = M5.Display;

    d.setFont(nullptr);
    d.setTextSize(1);

    d.fillRect(0, 0, 135, 26, BG);
    d.drawLine(0, 24, 135, 24, DARKGREY);

    d.setCursor(8, 8);
    d.print(leftText);

    d.setCursor(108, 8);
    d.print(wifiDots);
}

// -----------------------------
void drawRoutinePage(
    const String& headerLeft,
    const String& wifiDots,
    const std::vector<String>& steps,
    int countdown
) {
    auto& d = M5.Display;

    d.fillScreen(BG);
    drawHeader(headerLeft, wifiDots);

    d.setFont(&fonts::Font2);
    d.setTextSize(1);

    int y = 50;

    for (int i = 0; i < steps.size(); i++) {
        d.setCursor(10, y);

        String line =
            String(i + 1) +
            ". " +
            fitText(steps[i], 20);

        d.print(line);
        y += 34;
    }

    d.setFont(nullptr);
    d.setTextSize(1);

    d.drawLine(0, 235, 135, 235, DARKGREY);

    String nextText = "NEXT " + String(countdown) + "s";

int nextX = 135 - (nextText.length() * 6) - 8;

d.setCursor(nextX, 220);
d.print(nextText);
}

// -----------------------------
void drawClockPage(
    const String& wifiDots,
    const String& dateText,
    const String& timeText
) {
    auto& d = M5.Display;

    d.fillScreen(BG);
    drawHeader("CLOCK", wifiDots);

    // ===== date =====
    d.setFont(&fonts::Font2);
    d.setTextSize(1);

    int dateW = dateText.length() * 7;
    int dateX = (135 - dateW) / 2;

    // วาดซ้ำ offset 1px ให้ดูหนา
    d.setCursor(dateX, 58);
    d.print(dateText);

    d.setCursor(dateX + 1, 58);
    d.print(dateText);

    // ===== time =====
    d.setFont(&fonts::Font4);
    d.setTextSize(1);

    int timeW = timeText.length() * 18;
    int timeX = (135 - timeW) / 2 + 12;

    d.setCursor(timeX, 126);
    d.print(timeText);

    // ===== IP =====
    d.setFont(nullptr);
    d.setTextSize(1);
    d.setTextColor(LIGHTGREY, BG);

    String ip = getLocalIP();

    int ipX = (135 - ip.length() * 6) / 2;

    d.setCursor(ipX, 194);
    d.print(ip);

    d.setTextColor(DARKGREY, BG);
String version = "AuraClock v1.0";
int vx = (135 - version.length() * 6) / 2;

d.setCursor(vx, 214);
d.print(version);
d.setTextColor(WHITE, BG);
}

// -----------------------------
void drawInfoPage(
    const String& wifiDots,
    int batteryPercent,
    bool charging,
    bool lowBattery
) {
    auto& d = M5.Display;

    d.fillScreen(BG);
    drawHeader("INFO", wifiDots);

    if (lowBattery) {
        d.setFont(&fonts::Font2);
        d.setTextSize(1);

        d.setTextColor(RED, BG);

        String title = "BATTERY";
int titleX = (135 - title.length() * 7) / 2;

d.setCursor(titleX, 58);
d.print(title);

        drawLightning(40, 105, 3, RED);

        d.setTextColor(WHITE, BG);
        return;
    }

    d.setFont(&fonts::Font2);
    d.setTextSize(1);

    String title = "BATTERY";
int titleX = (135 - title.length() * 7) / 2;

d.setCursor(titleX, 58);
d.print(title);

    d.setFont(&fonts::Font4);
    d.setTextSize(1);

    String text = String(batteryPercent) + "%";

    int x =
        (135 - text.length() * 18) / 2;

    d.setCursor(x, 120);
    d.print(text);

    if (charging) {
        drawLightning(52, 170, 1, GREEN);
    }
}

// -----------------------------
void drawPortalGuidePage(
    const String& wifiDots
) {
    auto& d = M5.Display;

    d.fillScreen(BG);
    drawHeader("PORTAL", wifiDots);

    d.setFont(&fonts::Font2);
    d.setTextSize(1);

    d.setCursor(10, 52);
    d.print("1. Connect WiFi");

    d.setCursor(16, 82);
    d.print("AuraClock Setup");

    d.setCursor(10, 126);
    d.print("2. Open");

    d.setCursor(26, 156);
    d.print("clock.local");
}

// -----------------------------
void drawPortalActivePage(
    const String& wifiDots
) {
    auto& d = M5.Display;

    d.fillScreen(BG);
    drawHeader("ACTIVE", wifiDots);

    d.setFont(&fonts::Font2);
    d.setTextSize(1);

    d.setCursor(26, 80);
    d.print("PORTAL");

    d.setCursor(26, 112);
    d.print("ACTIVE");

    d.setCursor(12, 164);
    d.print("AuraClock Setup");

    d.setCursor(26, 190);
    d.print("clock.local");
}