# Big Time 1.4.0 — CloudPebble Config Test Fix 2

- CONFIG_TEST_MODE remains enabled.
- In emulator test mode, @rebble/clay now owns showConfiguration and webviewclosed automatically.
- The production manual configuration handlers no longer open/send a second config path during emulator testing.
- KiezelPay remains disabled in test mode.
- KiezelPay deinit is now also skipped in test mode, preventing shutdown faults from deinitializing a library that was never initialized.
- AppMessage inbox remains 1024 bytes.
- Publishing version remains 1.4.0.
