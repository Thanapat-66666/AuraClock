# AuraClock

Minimal premium smart clock for ESP32-S3.

## Features
- Wi-Fi captive portal setup
- remembers up to 5 networks
- auto reconnect
- NTP sync
- restore saved time
- mobile time sync fallback
- auto brightness
- sleep after 15s idle
- battery / charging indicator
- low battery alert

## Stack
- ESP32-S3
- PlatformIO
- Arduino
- M5Unified
- LittleFS

## Build
```bash
pio run
```

Upload firmware:

```bash
pio run -t upload
```

Upload portal:

```bash
pio run -t uploadfs
```
