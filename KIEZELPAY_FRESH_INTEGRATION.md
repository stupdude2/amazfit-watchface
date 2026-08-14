# Big Time v2.1.0 — Fresh KiezelPay Integration

This branch was rebuilt from the pre-KiezelPay backup and the personalized KiezelPay package for product 915728151.

## Licensing architecture

- KiezelPay is authoritative for PAID licensing.
- Big Time does not persist or invent a second paid-license cache.
- `KIEZELPAY_LICENSED` grants paid Pro access.
- `KIEZELPAY_CODE_AVAILABLE` represents an unlicensed purchase-code state.
- KiezelPay's built-in message display remains enabled.
- Periodic KiezelPay license checks remain enabled.
- KiezelPay's built-in timed trial is disabled because Big Time uses an opt-in trial.

## Big Time trial

Big Time manages its own 48-hour, opt-in Pro trial:
- It never starts automatically.
- It starts only after the user enables `Try Pro Free — 48 Hours` and saves Settings.
- The watch stores the original trial start time.
- PebbleKit JS stores the same start time in localStorage and syncs it after reinstall.
- A used/expired trial is reported separately from a never-used trial.
- Settings displays `Your Pro trial has ended.` after expiration.

## Free customization

Free users may change only:
- 12-hour / 24-hour time format
- Center 12-hour clock

The Free step goal is fixed at 5,000. All other customization requires an active Big Time trial or a KiezelPay paid license.

## KiezelPay test settings

The personalized `src/c/kiezelpay.c` is currently configured for TESTING:

- `KIEZELPAY_LOG_VERBOSE 1`
- `KIEZELPAY_TEST_MODE 1`
- `KIEZELPAY_DISABLE_TIME_TRIAL 1`
- `KIEZELPAY_DISABLE_MESSAGES 0`
- `KIEZELPAY_DISABLE_PERIODIC_CHECKS 0`
- `KIEZELPAY_LOW_MEMORY_MODE 0`

Before publishing:
- set `KIEZELPAY_LOG_VERBOSE` to `0`
- set `KIEZELPAY_TEST_MODE` to `0`
- set JS `KIEZELPAY_LOGGING` in `src/pkjs/index.js` to `false`

Do NOT change `KIEZELPAY_DISABLE_TIME_TRIAL` to 0 unless you intentionally want KiezelPay's automatic server-configured timed trial instead of Big Time's opt-in trial.

## AppMessage integration

KiezelPay uses `pebble-events`, so Big Time also subscribes to AppMessage with pebble-events and opens AppMessage only once.

Big Time message keys:
- 24 LICENSE_STATUS_REQUEST
- 25 LICENSE_STATUS
- 26 TRIAL_START
- 27 PURCHASE_REQUEST
- 28 TRIAL_SYNC_EPOCH

Entitlement status values sent to Settings:
- 0 Free / trial available
- 1 Big Time trial active
- 2 KiezelPay licensed Pro
- 3 Trial ended

## Test sequence

1. Install fresh.
2. Open Settings: expect Big Time Free page.
3. Verify only time format + center 12-hour are customizable.
4. Start the 48-hour trial, save, then reopen Settings: expect full Pro controls.
5. Delete/reinstall during trial: phone-side trial start should restore the same trial rather than start a new one.
6. With no test purchase, enable Unlock Pro with KiezelPay and Save: expect KiezelPay purchase code.
7. Complete a KiezelPay test purchase: expect KiezelPay confirmation.
8. Reopen Settings: expect Big Time Pro plus full controls.
9. Delete the KiezelPay test purchase and allow the test-mode periodic recheck to occur: expect KiezelPay to return the device to an unlicensed/code state.
10. Reinstall with an existing valid purchase: use Unlock Pro; KiezelPay may restore the license or present a code depending on its own recognized-purchase state.
