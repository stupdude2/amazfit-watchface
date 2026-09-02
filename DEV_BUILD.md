# Big Time v1.4.2 — CloudPebble Screenshot Build

Dedicated emulator/configuration build for collecting screenshots in CloudPebble/RePebble.

Includes the complete v1.4.2 feature set:
- Tap/Shake visibility support for Top Bar and Bottom Bar.
- Combined Tap/Shake/Backlight visibility option.
- Step Progress Bar style and visibility as separate settings.
- Wrist-gesture data reveal independent of whether ambient-light logic actually illuminates the backlight.
- Optional expandable digital clock.
- Matching 6 px expanded digital/analog outer edge margins.
- Immediate layout reconciliation after saved settings.

Screenshot/testing behavior:
- CONFIG_TEST_MODE = true in PebbleKit JS.
- CONFIG_TEST_MODE = 1 on the watch.
- Pro entitlement is forced on so every configuration control is available.
- KiezelPay is not initialized in test mode.
- Clay automatic configuration handling is enabled for CloudPebble/RePebble compatibility.
- Package version remains 1.4.2.
- UUID unchanged: ced51275-d445-4b7e-89f6-9e41110ed4da

DO NOT publish this build to the Pebble Store. Use the production 1.4.2 package for release.
