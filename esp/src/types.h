#pragma once

enum class ClockPage {
    ROUTINE,
    CLOCK,
    INFO,
    PORTAL_GUIDE,
    PORTAL_ACTIVE
};

enum class Mode {
    AUTO,
    MORNING,
    NIGHT
};

enum class WifiState {
    CONNECTING,
    CONNECTED,
    FAILED,
    AP_MODE
};

enum class RoutineType {
    MORNING,
    DAILY,
    PRE_ACTIVITY,
    LATE_NIGHT
};

enum class ButtonAction {
    NONE,
    NEXT_PAGE,
    OPEN_PORTAL_GUIDE,
    START_PORTAL,
    RESET_WIFI
};