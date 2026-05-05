#include "storage_manager.h"
#include <Preferences.h>

static Preferences prefs;

static const int MAX_WIFI = 5;

// --------------------
void initStorage() {
    prefs.begin("auraclock", false);
}

// ==================== WIFI ====================
std::vector<WiFiProfile> getSavedWiFiList() {
    std::vector<WiFiProfile> list;

    for (int i = 0; i < MAX_WIFI; i++) {
        String ssidKey = "ssid" + String(i);
        String passKey = "pass" + String(i);

        String ssid = prefs.getString(ssidKey.c_str(), "");
        String pass = prefs.getString(passKey.c_str(), "");

        if (ssid.length() > 0) {
            list.push_back({
                ssid,
                pass
            });
        }
    }

    return list;
}

// --------------------
bool hasSavedWiFi() {
    return !getSavedWiFiList().empty();
}

// --------------------
void saveWiFi(
    const String& ssid,
    const String& password
) {
    auto list = getSavedWiFiList();

    for (auto& item : list) {
        if (item.ssid == ssid) {
            item.password = password;

            for (int i = 0; i < list.size(); i++) {
                prefs.putString(
                    ("ssid" + String(i)).c_str(),
                    list[i].ssid
                );

                prefs.putString(
                    ("pass" + String(i)).c_str(),
                    list[i].password
                );
            }

            return;
        }
    }

    list.push_back({
        ssid,
        password
    });

    while (list.size() > MAX_WIFI) {
        list.erase(list.begin());
    }

    clearWiFi();

    for (int i = 0; i < list.size(); i++) {
        prefs.putString(
            ("ssid" + String(i)).c_str(),
            list[i].ssid
        );

        prefs.putString(
            ("pass" + String(i)).c_str(),
            list[i].password
        );
    }
}

// --------------------
void clearWiFi() {
    for (int i = 0; i < MAX_WIFI; i++) {
        prefs.remove(("ssid" + String(i)).c_str());
        prefs.remove(("pass" + String(i)).c_str());
    }
}

// ==================== TIME ====================
void saveLastEpoch(time_t epoch) {
    prefs.putULong64("epoch", (uint64_t)epoch);
}

// --------------------
time_t getLastEpoch() {
    return (time_t)prefs.getULong64("epoch", 0);
}

// --------------------
bool hasSavedTime() {
    return prefs.isKey("epoch");
}

// --------------------
void clearSavedTime() {
    prefs.remove("epoch");
}