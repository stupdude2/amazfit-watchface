# Big Time 1.4.4 Production Release

Publishable production build based on the validated 3.3.3 Tap/Touch fix codebase.

Includes:
- Pebble AccelerometerService tap/shake support.
- Pebble Time 2 touchscreen tap support for Tap/Shake visibility.
- Tap/Shake-only visibility works independently of Backlight on Tap / Backlight Motion system settings.
- Tap/Shake/Backlight mode supports either interaction and can request normal Pebble backlight interaction.
- All prior bar visibility, Step Bar, expandable digital clock, immediate settings apply, analog/free-mode, and Pro trial settings fixes.

Production safeguards:
- CONFIG_TEST_MODE is OFF in C and JS.
- KiezelPay production integration is enabled.
- KiezelPay verbose logging is OFF.
- Existing Big Time UUID retained.
