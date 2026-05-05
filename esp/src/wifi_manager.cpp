#include "wifi_manager.h"
#include "storage_manager.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <time.h>
#include <sys/time.h>

static WebServer server(80);
static DNSServer dns;

static bool portalRunning = false;
static bool wifiConnected = false;

static String dots = "..-";
static unsigned long lastAnim = 0;
static int animFrame = 0;
static unsigned long lastReconnectTry = 0;
static unsigned long lastBackupSave = 0;

// -----------------------------
bool hasValidTime() {
    time_t now = time(nullptr);
    return now > 1700000000;
}

// -----------------------------
void syncTimeFromPhone(time_t epoch) {
    timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);

    saveLastEpoch(epoch);
}

// -----------------------------
void restoreSavedTime() {
    if (!hasSavedTime()) return;

    time_t saved = getLastEpoch();
    if (saved <= 0) return;

    timeval tv;
    tv.tv_sec = saved;
    tv.tv_usec = 0;

    settimeofday(&tv, nullptr);
}

// -----------------------------
void saveTimeBackup() {
    if (!hasValidTime()) return;

    unsigned long nowMs = millis();

    if (nowMs - lastBackupSave >= 1800000UL) {
        lastBackupSave = nowMs;
        saveLastEpoch(time(nullptr));
    }
}

// -----------------------------
static void serveFile(const char* path, const char* type) {
    File file = LittleFS.open(path, "r");

    if (!file) {
        server.send(404, "text/plain", "Not found");
        return;
    }

    server.streamFile(file, type);
    file.close();
}

// -----------------------------
static void handleRoot() {
    serveFile("/index.html", "text/html");
}

static void handleCSS() {
    serveFile("/style.css", "text/css");
}

static void handleJS() {
    serveFile("/app.js", "application/javascript");
}

// -----------------------------
static void handleScan() {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();

    WiFi.mode(WIFI_AP_STA);

    int n = WiFi.scanNetworks();

    for (int i = 0; i < n; i++) {
        JsonObject item = arr.createNestedObject();
        item["ssid"] = WiFi.SSID(i);
        item["rssi"] = WiFi.RSSI(i);
    }

    String out;
    serializeJson(arr, out);

    server.send(200, "application/json", out);
}

static void handleSaved() {
    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.to<JsonArray>();

    auto saved = getSavedWiFiList();

    for (auto& item : saved) {
        arr.add(item.ssid);
    }

    String out;
    serializeJson(arr, out);

    server.send(
        200,
        "application/json",
        out
    );
}

static void handleTimeSync() {
    DynamicJsonDocument doc(512);

    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }

    time_t epoch = doc["epoch"] | 0;

    if (epoch > 0) {
        syncTimeFromPhone(epoch);
    }

    server.send(200, "application/json", "{\"ok\":true}");
}

// -----------------------------
static bool connectNow(
    const String& ssid,
    const String& pass
) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED) {
        delay(250);

        if (millis() - start > 10000) {
            return false;
        }
    }

    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    delay(1500);

    if (hasValidTime()) {
        saveLastEpoch(time(nullptr));
    }

    return true;
}

// -----------------------------
static void handleConnect() {
    DynamicJsonDocument doc(1024);

    auto err = deserializeJson(doc, server.arg("plain"));

    if (err) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }

    String ssid = doc["ssid"] | "";
    String password = doc["password"] | "";

    bool ok = connectNow(ssid, password);

    if (!ok) {
        server.send(200, "application/json", "{\"ok\":false}");
        return;
    }

    saveWiFi(ssid, password);

    server.send(200, "application/json", "{\"ok\":true}");

    delay(1200);
    ESP.restart();
}

// -----------------------------
static void setupPortalServer() {
    LittleFS.begin(true);

    dns.start(53, "*", WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/style.css", handleCSS);
    server.on("/app.js", handleJS);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/saved", HTTP_GET, handleSaved);
    server.on("/connect", HTTP_POST, handleConnect);
    server.on("/time", HTTP_POST, handleTimeSync);

    server.onNotFound([]() {
        server.sendHeader("Location", "http://clock.local/", true);
        server.send(302, "text/plain", "");
    });

    server.begin();
}

// -----------------------------
void startPortal() {
    WiFi.disconnect(true, true);
    delay(300);

    WiFi.mode(WIFI_AP);
    WiFi.softAP("AuraClock Setup");

    setupPortalServer();

    portalRunning = true;
    wifiConnected = false;
}

// -----------------------------
bool connectKnownWiFi() {
    auto savedList = getSavedWiFiList();

    if (savedList.empty()) {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(300);

    int found = WiFi.scanNetworks();

    if (found <= 0) {
        return false;
    }

    String bestSSID = "";
    String bestPass = "";
    int bestRSSI = -999;

    for (int i = 0; i < found; i++) {
        String scannedSSID = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);

        for (auto& saved : savedList) {
            if (saved.ssid == scannedSSID && rssi > bestRSSI) {
                bestRSSI = rssi;
                bestSSID = saved.ssid;
                bestPass = saved.password;
            }
        }
    }

    if (bestSSID == "") {
        return false;
    }

    return connectNow(bestSSID, bestPass);
}

// -----------------------------
void initWiFi() {
    initStorage();

    restoreSavedTime();

    if (!hasSavedWiFi()) {
        startPortal();
        return;
    }

    if (!connectKnownWiFi()) {
        startPortal();
        return;
    }

    wifiConnected = true;
    portalRunning = false;
}

// -----------------------------
void handlePortal() {
    saveTimeBackup();

    if (portalRunning) {
        dns.processNextRequest();
        server.handleClient();
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        return;
    }

    wifiConnected = false;

    unsigned long now = millis();

    if (now - lastReconnectTry >= 30000) {
        lastReconnectTry = now;
        connectKnownWiFi();
    }
}

// -----------------------------
bool isPortalRunning() {
    return portalRunning;
}

// -----------------------------
String getLocalIP() {
    if (portalRunning) return WiFi.softAPIP().toString();
    if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString();
    return "--.--.--.--";
}

// -----------------------------
String getWifiDots() {
    if (portalRunning) return "ooo";

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        return "...";
    }

    wifiConnected = false;

    unsigned long now = millis();

    if (now - lastAnim > 350) {
        lastAnim = now;
        animFrame = (animFrame + 1) % 3;

        switch (animFrame) {
            case 0: dots = "..-"; break;
            case 1: dots = ".--"; break;
            case 2: dots = "..."; break;
        }
    }

    return dots;
}