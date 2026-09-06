# Big Time 1.4.6 Production Release

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


## Purchase screen readability test (1.4.8)
- Restores the familiar KiezelPay-style green time bar, light-gray body, and black code panel.
- Uses Gothic 24 Bold for KZL.IO/CODE so the full URL fits while remaining noticeably larger than stock.
- Restores a larger Gothic 24 instruction line.
- Uses a large white Bitham 42 Bold purchase code on black.
- Corrects the custom renderer to display KiezelPay's numeric purchase code instead of an incorrect base-36 conversion.

## Purchase screen test 1.4.9
- Keeps the large white numeric KiezelPay code on the black panel.
- Increases BIG TIME PRO to Gothic 24 Bold.
- Increases KZL.IO/CODE to Gothic 28 Bold while keeping the stock-style font.
- Restores the original lighter green KiezelPay-style status bar using GColorDarkGreen.
- Keeps Enter code below: at Gothic 24, matching the previous readable size.
