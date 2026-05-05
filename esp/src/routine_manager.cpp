#include "routine_manager.h"
#include <M5Unified.h>
#include <time.h>

static bool altPage = false;
static unsigned long lastSwap = 0;

// ---------- helpers ----------
static tm* nowLocal() {
    static tm info;

    time_t now = time(nullptr);
    localtime_r(&now, &info);

    return &info;
}

static bool inRange(int total, int start, int end) {
    return total >= start && total <= end;
}

static bool isMorning(int total) {
    return inRange(total, 300, 960); // 05:00 - 16:00
}

static bool isPreActivity(int total) {
    return inRange(total, 985, 1320); // 16:25 - 22:00
}

static bool isLateNight(int total) {
    return inRange(total, 1350, 1440); // 22:30 - 24:00
}

// ---------- day ----------
String getDayTodayText() {
    tm* t = nowLocal();

    const char* days[] = {
        "SUN","MON","TUE","WED","THU","FRI","SAT"
    };

    return String(days[t->tm_wday]) + " - TODAY";
}

// ---------- time ----------
String getTimeString() {
    tm* t = nowLocal();

    char buf[6];
    sprintf(buf, "%02d:%02d", t->tm_hour, t->tm_min);

    return String(buf);
}

String getThaiDateString() {
    tm* t = nowLocal();

    char buf[32];
    sprintf(
        buf,
        "%02d / %02d / %04d",
        t->tm_mday,
        t->tm_mon + 1,
        t->tm_year + 2443
    );

    return String(buf);
}

// ---------- wifi ----------


// ---------- battery ----------
static int smoothBattery = 100;

int getBatteryPercent() {
    int raw = M5.Power.getBatteryLevel();

    if (raw < 0) raw = 0;
    if (raw > 100) raw = 100;

    // smooth
    smoothBattery = (smoothBattery * 8 + raw * 2) / 10;

    return smoothBattery;
}

bool isCharging() {
    return M5.Power.isCharging();
}

bool isLowBattery() {
    return getBatteryPercent() < 15;
}

// ---------- daily routine ----------
static std::vector<String> getDailyRoutine(int day) {
    switch (day) {
        case 1:
            return {
                "Bioderma",
                "Face Cleanser",
                "Adapalene 0.1%",
                "Dragon Blood"
            };

        case 2:
            return {
                "Bioderma",
                "Face Cleanser",
                "Benzoyl Peroxide",
                "Dragon Blood"
            };

        case 3:
            return {
                "Face Cleanser",
                "Niacinamide 10%"
            };

        case 4:
            return {
                "Bioderma",
                "Face Cleanser",
                "Adapalene 0.1%",
                "Dragon Blood"
            };

        case 5:
            return {
                "Face Cleanser",
                "Benzoyl Peroxide"
            };

        case 6:
            return {
                "Face Cleanser",
                "Rest"
            };

        default:
            return {
                "Face Cleanser",
                "Adapalene 0.1%",
                "Dragon Blood"
            };
    }
}

// ---------- routine ----------
std::vector<String> getRoutineSteps() {
    tm* t = nowLocal();

    int total = (t->tm_hour * 60) + t->tm_min;

    unsigned long ms = millis();

    if (ms - lastSwap >= 10000) {
        altPage = !altPage;
        lastSwap = ms;
    }

    // morning
    if (isMorning(total)) {
        return {
            "Face Cleanser",
            "Niacinamide 10%",
            "Sunscreen"
        };
    }

    // pre activity loop
    if (isPreActivity(total) && altPage) {
        return {
            "Face Cleanser",
            "Niacinamide 10%"
        };
    }

    // late night loop
    if (isLateNight(total) && altPage) {
        return {
            "Bioderma",
            "Face Cleanser",
            "Adapalene / Benzac"
        };
    }

    // normal daily
    return getDailyRoutine(t->tm_wday);
}

// ---------- countdown ----------
int getCountdown() {
    return 10 - ((millis() % 10000) / 1000);
}