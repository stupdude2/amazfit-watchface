# Big Time 1.4.0 — CloudPebble Config Test Fix 2

- CONFIG_TEST_MODE remains enabled.
- In emulator test mode, @rebble/clay now owns showConfiguration and webviewclosed automatically.
- The production manual configuration handlers no longer open/send a second config path during emulator testing.
- KiezelPay remains disabled in test mode.
- KiezelPay deinit is now also skipped in test mode, preventing shutdown faults from deinitializing a library that was never initialized.
- AppMessage inbox remains 1024 bytes.
- Publishing version remains 1.4.0.

## CloudPebble config test fix 3
- Added `WEATHER_REFRESH` as AppMessage key 56 in `package.json`.
- Clay automatic event handling serializes every config item with a `messageKey`; the phone-only `WEATHER_REFRESH` setting previously had no numeric AppMessage mapping, producing `Unknown message key 'NaN'` in the RePebble emulator and aborting the entire settings send.
- The watch ignores key 56; weather refresh remains phone-side. This mapping exists only so Clay can serialize the complete settings response successfully in test mode.
