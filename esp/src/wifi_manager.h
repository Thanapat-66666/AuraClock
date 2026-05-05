#pragma once
#include <Arduino.h>

// boot
void initWiFi();

// portal
void startPortal();
void handlePortal();
bool isPortalRunning();

// reconnect
bool connectKnownWiFi();

// time
void restoreSavedTime();
void saveTimeBackup();
bool hasValidTime();
void syncTimeFromPhone(time_t epoch);

// info
String getLocalIP();
String getWifiDots();