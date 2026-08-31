# Big Time 3.3.1 — Production Publishing Build

Production package based on the approved 1.4.0 CloudPebble config-test/free-analog codebase.

- Analog clock face is available in Free.
- Analog second hand option is available in Free.
- Analog hand color customization remains Pro-only.
- Seconds Colon, Bluetooth Colon, and Time Style are disabled in Settings when Analog is selected.
- CloudPebble CONFIG_TEST_MODE is disabled in both C and PebbleKit JS.
- KiezelPay production licensing remains enabled; verbose KiezelPay logging is disabled.
- WEATHER_REFRESH AppMessage key 56 is retained so Clay settings serialize correctly.
- Package version: 3.3.1.
