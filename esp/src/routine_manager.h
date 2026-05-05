#pragma once
#include <Arduino.h>
#include <vector>

// time
String getDayTodayText();
String getTimeString();
String getThaiDateString();

// wifi
String getWifiDots();

// battery
int getBatteryPercent();
bool isCharging();
bool isLowBattery();

// routine
std::vector<String> getRoutineSteps();

// countdown
int getCountdown();