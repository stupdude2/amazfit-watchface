# Big Time 1.4.0 — CloudPebble Config Test Fix

Dedicated emulator build for testing all Pro settings in CloudPebble/RePebble.

Changes from the first config-test build:
- Pro edition state is forced before config.js is first loaded, avoiding stale Free config caching in pypkjs.
- KiezelPay is not initialized on either JS or watch side while CONFIG_TEST_MODE is enabled.
- Trial/license restore traffic is suppressed in test mode.
- Watch starts permanently Pro in test mode and never applies Free defaults.
- AppMessage inbox increased to 1024 bytes for the full Pro Clay settings dictionary.

Do not publish this build. Disable CONFIG_TEST_MODE and restore normal licensing before release.
