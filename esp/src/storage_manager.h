#pragma once
#include <Arduino.h>
#include <vector>

struct WiFiProfile {
    String ssid;
    String password;
};

// init
void initStorage();

// ---------------- WiFi ----------------
std::vector<WiFiProfile> getSavedWiFiList();
bool hasSavedWiFi();

void saveWiFi(
    const String& ssid,
    const String& password
);

void clearWiFi();

// ---------------- Time backup ----------------
void saveLastEpoch(time_t epoch);
time_t getLastEpoch();
bool hasSavedTime();
void clearSavedTime();