# Big Time v1.4.1 — Tap/Shake Visibility Test

Development/test build based on the v1.4.1 wrist-gesture data build.

Changes in this build:
- Added Pebble Accelerometer Tap Service support. Pebble documents these tap events as firing for a significant tap or shake of the watch.
- Top Bar and Bottom Bar visibility now offer: Always Visible, Show With Backlight, Show With Tap/Shake, Show With Tap/Shake/Backlight, and Always Hidden.
- Tap/Shake visibility is independent of the physical backlight and uses the learned system backlight duration when available (5-second fallback until learned).
- Step Progress Bar style and visibility are now separate controls.
- Step Progress Bar visibility uses the same five choices as the Top and Bottom bars.
- Existing legacy Step Bar modes are migrated to the equivalent Style + Visibility combination on both the watch and Clay settings page.
- Existing custom Wrist Raise Backlight behavior remains intact and can coexist with the built-in Tap/Shake event service.

Publishing safeguards remain unchanged:
- CONFIG_TEST_MODE = false
- KiezelPay production integration enabled
- package version remains 1.4.1 for testing
