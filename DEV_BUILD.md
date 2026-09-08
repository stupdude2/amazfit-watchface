# Big Time 1.4.8 Production Release

## Changes
- Added Quiet Time status to the digital clock: the upper colon dot is outlined while Quiet Time is active.
- Added analog status indicators: the 9 o’clock dash outlines for Quiet Time and the 3 o’clock dash outlines when Bluetooth is disconnected.
- Replaced missing heart-rate “--” values with an outlined heart icon.
- Added a compact steps icon beside hidden-label step values when the count is three digits or fewer, positioned on the outside edge of left/right data slots.
- Added UV Index as a configurable data item using the existing Open-Meteo weather refresh/cache pipeline.
- Added ISO Week of Year as a configurable data item.

## Production safeguards
- CONFIG_TEST_MODE remains disabled.
- KiezelPay production licensing remains enabled.
- Existing application UUID is retained.
