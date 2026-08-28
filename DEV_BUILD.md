# Big Time development build 22

This build replaces the experimental tap/backlight inference from dev-19 through dev-21 with Pebble SDK 4.33's dedicated touchscreen gesture recognizer.

- Wrist Raise: Backlight Only still turns on the system backlight without revealing conditional data.
- A real touchscreen tap is captured by `tap_recognizer_create()` attached to the watchface window.
- The tap explicitly reveals backlight-only header/footer/step data and requests normal backlight interaction whether the light was already on or off.
- BacklightService no longer attempts to infer taps from repeated ON events, so normal system backlight behavior remains untouched.
- The reveal remains latched only for the current backlight session and resets when the backlight turns off.
