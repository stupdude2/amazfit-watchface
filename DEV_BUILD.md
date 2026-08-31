# Big Time 1.4.0 — CloudPebble Config Test Build

This build is based on the approved 1.4.0 release code and enables CONFIG_TEST_MODE for emulator/web configuration testing.

- All Pro controls are exposed in Clay without a KiezelPay entitlement.
- Pro AppMessage settings are accepted by the watch/emulator so changes can be tested end-to-end.
- The watch reports a Pro entitlement to the settings UI while test mode is active.
- KiezelPay production identifiers remain present, but entitlement enforcement is bypassed only by the explicit test-mode flags.
- Package version remains 1.4.0.

IMPORTANT: Do not publish this ZIP. Publishing builds must set CONFIG_TEST_MODE to false/0 in both src/pkjs/index.js and src/c/main.c.
