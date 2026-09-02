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

## Digital clock expansion test
- Added Pro option: **Expand Digital Clock**.
- When enabled, the digital clock reclaims vertical space whenever the top bar, bottom bar, or progress bar is not currently visible.
- The clock frame animates using the same 260 ms ease-in/out behavior as the analog face.
- Seven-segment digit height stretches with the available clock frame while the established horizontal geometry remains unchanged.
- Gesture/backlight/Tap-Shake bar visibility continues to contract and expand the clock dynamically.

## Immediate settings application audit
- Fixed Step Progress Bar Visibility not always applying immediately after Save.
- Root cause: STEP BAR VISIBILITY is stored outside the main WatchfaceSettings record, so the post-save delta optimization could incorrectly clear the relayout flag.
- Post-save structural reconciliation now includes Step Bar Visibility, Analog/Digital face changes, Expand Digital Clock, and all time-zone selectors in addition to the main settings record.
- Top/Bottom visibility, Step style/visibility, clock format/layout, clock-face switching, and expanded-clock geometry are now reconciled from the final parsed settings state immediately after Save.
