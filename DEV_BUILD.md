# Big Time 1.4.5 Production Release

Publishable production build based on the validated 1.4.4 release.

## Changes
- Renamed the visibility option from **Tap/Shake** to **Shake** so the setting accurately describes the input watchfaces can reliably receive.
- Removed the ineffective Pebble Time 2 touchscreen-touch reveal path.
- **Show With Shake** continues to use Pebble's AccelerometerService event and works independently of the system backlight settings.
- **Show With Shake/Backlight** responds to either the accelerometer gesture or the normal backlight interaction.

## Production safeguards
- CONFIG_TEST_MODE is disabled.
- Production KiezelPay integration remains enabled.
- Verbose KiezelPay logging is disabled.
- Existing Big Time application UUID is retained.
