# Amazfit Bip Port — Pebble Time 2 Watchface

A faithful port of the Amazfit Bip digital watchface to the Pebble Time 2 (Basalt platform, 144×168 px color display).

## Why Pebble C

Pebble C is the only correct choice for a watchface on this hardware:

- **PebbleKit JS** runs on the phone. It handles Bluetooth messaging and background fetch. It cannot draw a watchface.
- **Pebble Package** is a library distribution format, not an application type.
- **Pebble C** runs on the watch, owns the display, and has direct access to tick timers, health sensors, and layer drawing. That is what a watchface needs.

## Layout (144 × 168 px)

```
┌──────────────────────────────┐  y=0
│  SUN   [ 7 ]   JAN           │  28px — blue header (GColorCobaltBlue)
├──────────────────────────────┤  y=28
│                              │
│           0:00               │  108px — clock area, black bg
│                              │       — LECO 42 digit font
├──────────────────────────────┤  y=136
│ ☁  WEATHER  │ HR  │  STEPS   │  32px — footer, black bg, blue top rule
│             │ 62  │  1000    │
└──────────────────────────────┘  y=168
```

## Features

| Feature | Implementation |
|---|---|
| Time | `tick_timer_service`, MINUTE_UNITS |
| Day / Date / Month | `strftime` with uppercase conversion |
| Date box | White filled rect drawn in header layer callback |
| Heart rate | `health_service_peek_current_value(HealthMetricHeartRateBPM)` |
| Step count | `health_service_sum_today(HealthMetricStepCount)` |
| Weather icon | Drawn procedurally in footer layer callback (blue circle + white cloud) |
| Platform guard | `#if defined(PBL_HEALTH)` keeps it compile-safe on non-health builds |

## Building

```bash
# Install Pebble SDK (or use CloudPebble)
pebble build
pebble install --phone <YOUR_PHONE_IP>
```

Or paste the `src/main.c` and `package.json` into [CloudPebble](https://cloudpebble.net).

## Customisation notes

- **Live weather data**: hook up AppMessage in `src/main.c` and send temperature from a PebbleKit JS companion script in `src/js/app.js`.
- **Weather icon**: swap the procedural drawing in `footer_update_proc` for a `GBitmap` resource to match the Amazfit cloud icon exactly. Add the PNG to `resources/images/` and register it in `package.json` under `resources.media`.
- **24h vs 12h**: change `%k:%M` to `%I:%M` and add an AM/PM indicator layer.
- **Font**: `FONT_KEY_LECO_42_NUMBERS` is the closest built-in to the Amazfit segmented display style. For an exact match, embed a custom `.ttf` as a resource font.


## v1.2.4 footer reveal modes

Pro bottom-bar visibility now supports Always Visible, Double Wrist Tap, Single Screen Tap (Pebble Time 2 touch), Show While Backlight Is On, and Off. The double-tap timing window is also relaxed to 1 second.


## v1.2.4
- Simplified top/bottom bar visibility to Always Visible or Show With Backlight.
- Added configurable top bar visibility.
- Backlight-controlled bars resync with `light_is_on()` whenever the watchface regains focus, so they remain visible when returning from menus while the backlight is already on.
- Simplified Bluetooth option labels to “Bluetooth”.


### v1.2.4
- Step progress bar can be placed below or above the time in mirrored or left-to-right styles.
- When the step bar is hidden, the clock automatically expands into the freed space and vertically centers the time.


### v1.3.1 layout refinement
- Backlight-only step bars reserve their layout space so the clock does not move when the light changes.
- Fully hidden step bar expands the clock area and biases the time 3 px upward.


## v1.3.1 customization additions
- Daily Step Goal slider: 1,000–30,000 in 500-step increments.
- Fahrenheit / Celsius temperature unit selection.
- Independent clock digit color picker.

## Background color and automatic contrast

Pro builds now include a Background Color picker. Text and native footer icons that sit directly on the background automatically switch between light and dark foreground colors using Pebble's `gcolor_legible_over()`. Weather icons are tinted at runtime from the existing white RGBA assets, preserving their transparency while automatically matching the light/dark foreground selected for the current background. Accent-box text continues to contrast against the accent color, while Clock Color remains independently user-selectable.


## v2.0.17 clock auto-contrast refinement

When the clock color is pure white or pure black, changing the background color now automatically switches the clock between white and black using Pebble's legibility helper. Any non-neutral custom clock color remains unchanged.


## v2.0.17
Top and bottom bars now share customizable content choices. Day of Week, Date, and Month are available in bottom slots, while Weather, Steps, Battery, Heart Rate, Bluetooth, Day of Week, Date, and Month can be assigned to any top slot.


## v2.0.17
- Added Restore Default Settings button with confirmation dialog in Clay.
- Added Always Hidden modes for both Top Bar and Bottom Bar.
- Renamed the step progress bar's Hidden option to Always Hidden for consistency.

- Replaced native JavaScript restore confirmation/alert dialogs with an in-page confirmation card and status message.

- Shortened the restore confirmation action button to Yes for compact mobile layout.

- Constrained restore confirmation buttons to equal 92px widths for reliable mobile layout.

## v2.0.17
- Added selectable 12-hour / 24-hour clock format.
- Preserved the original 12-hour digit geometry unchanged.
- Added dedicated 24-hour geometry: 110px height, 12px strokes, 36px first-hour cell, 42px remaining digit cells, with the original 6px digit/colon spacing.
- Existing settings migrate forward with 12-hour mode as the default.

## v2.0.17
- 24-hour digit 1 now uses the same full-width seven-segment cell geometry as the other numerals.
- 12-hour digit geometry is unchanged.

## v2.0.17
- Increased 24-hour digit stroke thickness from 12px to 13px.
- Narrowed the three regular 24-hour digit cells from 42px to 41px, increasing the outer margins while preserving the existing 6px digit/colon gaps and 110px clock height.

## v2.0.17
- Increased 24-hour stroke thickness from 13px to 14px.
- Narrowed regular 24-hour digit cells from 41px to 40px, increasing the outer margins to about 5.5px per side.
- Applied the original 12-hour first-digit 1px inset to the leading 24-hour digit cell while retaining its wider geometry.

## v2.0.17
- 24-hour digit 1 keeps a full-width allocated cell but now uses the same deliberate left-biased glyph placement as the 12-hour clock.
- Regular 1s use the existing 8px ONE_X_OFFSET.
- The leading hour 1 uses the existing 1px H1_ONE_X offset.
- All 24-hour spacing, 14px stroke thickness, and outer margins remain unchanged.

## v2.0.17
- 24-hour leading digit 1 now uses the same 8px ONE_X_OFFSET as every other 1.
- 12-hour leading-digit geometry remains unchanged.

## v2.0.17
- Moved Time Format to the first setting under Appearance.

## v2.0.17
- Added Center 12 Hour Clock.
- When enabled, 1:00-9:59 centers the visible H:MM group.
- 10:00-12:59 keeps the original four-digit spacing.

## v2.0.17
- Disabled Center 12 Hour Clock in settings whenever 24 Hour Time Format is selected; it re-enables when returning to 12 Hour.
- All four 24-hour digit cells now use the same 39px width.
- Removed the special first-digit geometry/inset in 24-hour mode.
- Kept the 24-hour clock at the previous 189px total width with 14px strokes and existing digit/colon spacing.

## v2.0.17
- Bottom-bar Day of Week, Date, and Month now match the header calendar style.
- Calendar items use the large custom header font with no small label.
- Calendar values are centered horizontally and vertically in left, center, or right footer slots.
- Other footer item layouts are unchanged.

## v2.0.17
- Added left/right-only bar choices for Calories, Distance, Sunrise, Sunset, and High / Low Temp.
- Calories use Pebble Health active kilocalories for the current day.
- Distance uses Pebble Health walked distance; miles are shown with Fahrenheit and kilometers with Celsius.
- Sunrise/Sunset follow the selected 12/24-hour time format.
- High/Low uses the current temperature plus today's available 3-hour OpenWeather forecast points.
- Top and bottom center choices remain unchanged.

## v2.0.17
- Removed Day of Week and Month from top-center and bottom-center choices.
- Center Weather now displays TEMP as the label with a centered temperature value and no weather icon.
- Moved the top-center battery icon upward and matched its value layout to the bottom-center battery.

## v2.0.17
- Step progress bar now switches to the Accent Color when the daily step goal is reached.

## v2.0.17
- Added optional Raise to Wake modes: Off, Normal, and Sensitive.
- Uses 10 Hz accelerometer samples batched five at a time (two app callbacks per second).
- Detects a lowered/edge-on wrist followed by a stable face-toward-user orientation within a short gesture window.
- Ignores vibration-contaminated samples, rejects high-impact samples, and uses a cooldown to reduce false wakes.
- Triggers Pebble's system-managed short backlight with light_enable_interaction().

## v2.0.17
- Reworked Raise to Wake around an upright, wearer-facing read pose instead of a face-up pose.
- The detector no longer requires the wrist to begin from a hanging/down position.
- Any meaningful recent wrist motion can arm the detector; entering and briefly stabilizing in the read pose triggers the backlight.
- Uses Y-dominant / low-Z orientation thresholds, motion memory, stable-pose confirmation, and cooldown filtering.
- Raise logs now include x/y/z values for tuning on real hardware.

## v2.0.17
- Raise to Wake now preserves the Y-axis sign instead of using |Y|.
- Wearer-facing orientation requires negative Y, based on PT2 hardware logs.
- Mirror-image positive-Y orientation (watch flipped away) is rejected.

## v2.0.17
- Normal Raise to Wake now requires slightly more wrist movement (motion threshold 180 -> 210).
- Normal motion-memory window shortened from 1100 ms to 950 ms to reduce incidental triggers.
- Accelerometer remains at 10 Hz, but batching increased from 5 to 10 samples.
- App-side accelerometer callbacks drop from about 2 per second to about 1 per second, reducing CPU wakeups while retaining the same sensor sampling fidelity.
- Sensitive mode thresholds remain unchanged.

## v2.0.17
- Decoupled conditional UI visibility from the physical backlight LED.
- Successful Raise to Wake starts a 4-second logical interaction window even if ambient light suppresses the LED.
- PT2 screen Touchdown also starts the logical interaction window and requests the normal system backlight interaction.
- Top Bar, Bottom Bar, and backlight-only Step Bar now show whenever either the real backlight is on OR the logical interaction window is active.
- Added raise logs showing both logical interaction and physical light state for outdoor testing.

## v2.0.17 — Runtime Free / Pro licensing foundation
- Converted the project from separate Standard/Pro compile-time editions to one runtime-gated build.
- Free users: 5000-step target, Time Format (12/24 hour), and Center 12 Hour Clock only.
- Center 12 Hour Clock defaults ON for free users.
- All colors, weather units, bar customization, step-goal changes, progress layouts, and Raise to Wake require Pro.
- Pro restrictions are enforced in C so manually-crafted AppMessages cannot bypass the settings UI.
- Reserved PRO_LICENSE=24 and LICENSE_CHECK=25 for the KiezelPay license bridge.

## v2.0.17
- Forced Clay's full COLOR palette for Accent, Clock, and Background controls.
- Prevents PT2/Emery configuration from falling back to the black/white palette.

## v2.0.17
- Added missing Pebble package dependencies required by the KiezelPay integration:
  - kiezelpay-core ^2.0.17
  - pebble-events ^1.0.2
- Keeps the forced full COLOR Clay palette from v2.0.1.

## v2.0.17
- Disabled Clay's automatic configuration event handling.
- Added explicit showConfiguration handler using clay.generateUrl() + Pebble.openURL().
- Added explicit webviewclosed handler using clay.getSettings() + Pebble.sendAppMessage().
- Intended to prevent settings-button failures when Clay is used alongside the KiezelPay package stack.

## v2.0.17 — Real KiezelPay entitlement integration
- Integrated the official generated KiezelPay file for product 915728151.
- Enabled KiezelPay timed-trial support so the server-configured 48-hour trial can unlock Pro.
- KIEZELPAY_TRIAL_STARTED and KIEZELPAY_LICENSED now unlock Big Time Pro.
- KIEZELPAY_TRIAL_ENDED returns the watchface to Free mode.
- KiezelPay and Big Time now share AppMessage through pebble-events rather than competing for the single native callback.
- Added the official KiezelPay JavaScript companion initialization.
- Weather JS now ignores KiezelPay AppMessages instead of treating every package message as a weather request.
- Test mode and verbose KiezelPay logging remain enabled for purchase/trial testing.

## v2.0.17 — KiezelPay auto-purchase/state fix
- Disabled KiezelPay's automatic time-trial flow again.
- Opening Settings no longer starts a trial or purchase-code sequence.
- Removed persistent phone-side Pro caching as a source of truth.
- Settings now asks the watch for authoritative license status every time it opens.
- If the watch does not confirm Pro, the settings page fails closed to Free.
- A real KiezelPay LICENSED event is the only KiezelPay event that unlocks Pro.
- The optional 48-hour Pro trial will be implemented as an explicit user-started Big Time trial, not as KiezelPay's auto-start trial.

## v2.0.17 — Explicit trial and purchase controls
- Added a visible 48-hour Pro trial notice to Free settings.
- Added Try Pro Free toggle; trial begins only after enabling it and tapping Save.
- Added Unlock Pro with KiezelPay toggle; purchase code begins only after enabling it and tapping Save.
- Added Purchase / Restore Pro control while a trial is active.
- Big Time manages the explicit 48-hour trial itself so KiezelPay's automatic timed trial remains disabled.
- Trial state persists across watchface restarts.
- KiezelPay remains the authority for purchased/restored Pro licenses.
- New AppMessage keys: TRY_PRO_FREE=26 and UNLOCK_PRO=27.

## v2.0.17
- Fixed build error in TRY_PRO_FREE and UNLOCK_PRO handlers by passing the required fallback argument to tuple_to_int32().

## v2.0.17
- Refined black/white clock-background conflict handling.
- Selecting Black Clock while Background is Black now preserves the black clock and flips Background to White.
- Selecting White Clock while Background is White now preserves the white clock and flips Background to Black.
- Changing Background to match an existing black/white clock still flips the clock for contrast.
- Custom non-black/white clock colors remain unchanged.

## v2.0.17
- Settings Save now mirrors the watch's black/white clock/background conflict logic before values are sent.
- If Black Clock + Black Background is saved, Background is stored/sent as White.
- If White Clock + White Background is saved, Background is stored/sent as Black.
- This keeps Clay's remembered settings aligned with what the watch actually displays.
- On a fresh Free -> Pro unlock, the Pro step goal initializes to 5000 instead of reviving stale cached values from older sessions.

## v2.0.17
- Black/white clock/background conflict correction now happens live inside the Clay settings page.
- Selecting Black Clock while Background is Black visibly changes Background to White in settings.
- Selecting White Clock while Background is White visibly changes Background to Black in settings.
- Changing Background into a black/black or white/white conflict visibly changes Clock to the contrasting color.
- Added CURRENT_STEP_GOAL=28 watch-to-phone telemetry.
- Pro settings now seed the Step Goal control from the watch's actual current value.
- A newly unlocked Pro watch coming from Free therefore shows 5000 rather than stale cached values such as 14000.

## v2.0.17
- Removed all automatic Clock Color / Background Color swapping.
- Clock and background colors now remain exactly as selected, even if they match.
- Removed CURRENT_STEP_GOAL; AppMessage keys remain 0-27.
- Pro Step Goal initializes to 5000 on a new Free -> Pro unlock using the existing STEP_GOAL key (10).

## v2.0.17
- Settings title is now always Big Time.
- Added entitlement text directly below the title:
  - Free
  - Free Trial — Your Big Time Pro trial is currently active.
  - Pro — Thank you for purchasing Big Time Pro!
- Reused the existing PRO_LICENSE key (24) as a 3-state entitlement value; no new AppMessage key is required.
- PRO_LICENSE values are now 0=Free, 1=Free Trial, 2=Purchased/Restored Pro.

## v2.0.17
- Free Trial status now includes the actual remaining trial time in hours and minutes.
- Reuses the existing LICENSE_CHECK key (25) in the watch-to-phone status response; no new AppMessage key is required.

## v2.0.17 — Trial reinstall protection
- Mirrors the one-time 48-hour Pro trial to PebbleKit JS localStorage.
- PebbleKit JS localStorage survives watchapp uninstall/reinstall, so reinstalling no longer creates a fresh trial on the same phone.
- TRY_PRO_FREE (26) is reused to restore the original Unix expiration timestamp to the watch; no new AppMessage key is required.
- If the original trial is still active after reinstall, the watch resumes with the original remaining time.
- If the trial has expired, a reinstall cannot start another trial.

## v2.0.17
- Removed the “— Pro” suffix from settings section headings.
- Trial/Pro entitlement remains communicated by the status text beneath the Big Time title.

## v2.0.17 — Conditional bar state-machine fix
- Reworked logical backlight interaction state around an absolute expiration timestamp.
- Top bar, bottom bar, and backlight-only step bar can no longer remain visible solely because a stale interaction boolean survived a delayed/canceled timer.
- Every visibility check, backlight transition, focus return, and input-service refresh validates the interaction deadline.
- Replaced app_timer_reschedule() with cancel + fresh one-shot timer for each touch/raise interaction.
- Added debug logs for interaction start, timeout, physical backlight transitions, and focus returns.
- Preserves the sunlight behavior: a raise/touch can still show conditional data even when ambient-light logic suppresses the physical LED.

## v2.0.17
- Fixed the v2.0.16 build error by declaring interaction_should_be_visible() before update_stepbar_layout() uses it.
- Conditional-bar state-machine changes from v2.0.16 are otherwise unchanged.
