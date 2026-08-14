# Big Time — KiezelPay + opt-in Pro trial

Product ID: **915728151**

This project now contains the generated KiezelPay Pebble integration and a separate
watch-side 48-hour Pro trial. The trial is deliberately **not** started on install.

## User flow

1. Big Time installs in Free mode.
2. Settings shows **Try Pro Free — 48 Hours**.
3. The user turns it on and taps **Save Settings**. Only then is the trial start
   timestamp written to watch persistent storage.
4. All Pro features are enabled for 48 hours.
5. At expiry Big Time returns to Free defaults, but saves the user's Pro
   customization separately.
6. **Unlock Pro with KiezelPay** starts the purchase-code flow.
7. A valid KiezelPay license permanently unlocks Pro and restores the saved Pro setup.

## Security / authority

- Paid entitlement is validated by KiezelPay in C.
- Trial start/used/time state is persisted on the watch, not only in phone JS.
- Premium AppMessage settings remain gated in C.
- The phone only mirrors entitlement/trial state for the Settings UI.
- The merchant API key is not embedded in the PBW.

## IMPORTANT — test vs. Store release

The delivered 2.0.2 integration build is intentionally configured for purchase testing:

- `src/c/kiezelpay.c`: `KIEZELPAY_TEST_MODE 1`
- `src/c/kiezelpay.c`: `KIEZELPAY_LOG_VERBOSE 1`
- `src/pkjs/index.js`: `KIEZELPAY_LOGGING = true`

Before publishing to the Pebble Store, change them to:

- `KIEZELPAY_TEST_MODE 0`
- `KIEZELPAY_LOG_VERBOSE 0`
- `KIEZELPAY_LOGGING = false`

Keep `KIEZELPAY_DISABLE_TIME_TRIAL 1` in both test and production. Big Time owns
the opt-in 48-hour trial; KiezelPay's automatic timed-trial behavior must remain disabled.
