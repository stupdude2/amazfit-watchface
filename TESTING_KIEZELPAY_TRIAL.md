# Big Time 2.0.2 — KiezelPay / Pro trial test checklist

This archive is a **test build**. KiezelPay test mode and verbose logging are ON.

## Fresh free install

1. Install/sideload the watchface.
2. Confirm the default face uses a 5000-step goal.
3. Open Settings. Confirm only Time Format and Center 12 Hour Clock are freely configurable.
4. Confirm the trial has NOT started merely by installing/opening Settings.

## Opt-in trial

1. In Settings enable **Try Pro Free — 48 Hours**.
2. Tap **Save Settings**.
3. Reopen Settings. Confirm the complete Pro controls are visible and the trial-active message is shown.
4. Change several obvious Pro settings (accent/background, bar layout, step goal), then Save.
5. Restart the watchface and confirm those Pro settings persist.

## Purchase path

1. On a Free/expired installation, open Settings.
2. Enable **Unlock Pro with KiezelPay** and tap Save Settings.
3. Confirm a KiezelPay purchase code appears on the watch.
4. Complete a KiezelPay TEST purchase.
5. Confirm Pro unlocks and previously saved Pro settings return.

## Expiration testing

The production trial is fixed at 48 hours. For rapid local testing, temporarily change
`PRO_TRIAL_SECONDS` in `src/c/main.c` from `(48 * 60 * 60)` to e.g. `120`, rebuild,
and use a fresh app persistent-storage state. Restore 48 hours before release.

After expiration confirm:

- Big Time returns to Free defaults.
- Time Format and Center 12 Hour Clock remain user-configurable.
- The trial cannot be started a second time.
- Settings explains that the saved Pro setup will return after purchase.
- Completing a KiezelPay test purchase restores the saved Pro customization.

## Before Store release

Change:

- `KIEZELPAY_TEST_MODE` to `0` in `src/c/kiezelpay.c`
- `KIEZELPAY_LOG_VERBOSE` to `0` in `src/c/kiezelpay.c`
- `KIEZELPAY_LOGGING` to `false` in `src/pkjs/index.js`

Do **not** change `KIEZELPAY_DISABLE_TIME_TRIAL`; it must remain `1` because Big Time
uses its own explicit opt-in trial rather than KiezelPay's automatic timed trial.


## Reinstall / repeat purchase test (v2.0.2)

If a previous KiezelPay test purchase was completed and the watchface is reinstalled, use **Unlock Pro with KiezelPay** and Save Settings. Big Time now waits until the settings AppMessage is complete, cancels any stale KiezelPay purchase session, and then starts a fresh purchase/status request. Expected result: KiezelPay either restores licensed status or shows a fresh code.


## Reinstall / stale entitlement test (v2.0.3)

1. Complete a KiezelPay test purchase and verify Pro controls appear.
2. Remove Big Time from the watch and reinstall it.
3. Open Settings before doing anything else.
4. If KiezelPay has not yet restored the paid license, Settings must open in Free mode and show the KiezelPay unlock action; stale Pro controls must not appear.
5. If KiezelPay validates the previous license, Settings may open in Pro mode.
6. After a 48-hour trial has ever been started, reinstalling Big Time should not offer a new `Try Pro Free` action on the same phone; it should show the trial as used/expired unless KiezelPay reports a permanent license.
7. Confirm a Free watch ignores crafted/saved Pro customization values.
