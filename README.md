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


## v2.0.2 clock auto-contrast refinement

When the clock color is pure white or pure black, changing the background color now automatically switches the clock between white and black using Pebble's legibility helper. Any non-neutral custom clock color remains unchanged.


## v2.0.2
Top and bottom bars now share customizable content choices. Day of Week, Date, and Month are available in bottom slots, while Weather, Steps, Battery, Heart Rate, Bluetooth, Day of Week, Date, and Month can be assigned to any top slot.


## v2.0.2
- Added Restore Default Settings button with confirmation dialog in Clay.
- Added Always Hidden modes for both Top Bar and Bottom Bar.
- Renamed the step progress bar's Hidden option to Always Hidden for consistency.

- Replaced native JavaScript restore confirmation/alert dialogs with an in-page confirmation card and status message.

- Shortened the restore confirmation action button to Yes for compact mobile layout.

- Constrained restore confirmation buttons to equal 92px widths for reliable mobile layout.

## v2.0.2
- Added selectable 12-hour / 24-hour clock format.
- Preserved the original 12-hour digit geometry unchanged.
- Added dedicated 24-hour geometry: 110px height, 12px strokes, 36px first-hour cell, 42px remaining digit cells, with the original 6px digit/colon spacing.
- Existing settings migrate forward with 12-hour mode as the default.

## v2.0.2
- 24-hour digit 1 now uses the same full-width seven-segment cell geometry as the other numerals.
- 12-hour digit geometry is unchanged.

## v2.0.2
- Increased 24-hour digit stroke thickness from 12px to 13px.
- Narrowed the three regular 24-hour digit cells from 42px to 41px, increasing the outer margins while preserving the existing 6px digit/colon gaps and 110px clock height.

## v2.0.2
- Increased 24-hour stroke thickness from 13px to 14px.
- Narrowed regular 24-hour digit cells from 41px to 40px, increasing the outer margins to about 5.5px per side.
- Applied the original 12-hour first-digit 1px inset to the leading 24-hour digit cell while retaining its wider geometry.

## v2.0.2
- 24-hour digit 1 keeps a full-width allocated cell but now uses the same deliberate left-biased glyph placement as the 12-hour clock.
- Regular 1s use the existing 8px ONE_X_OFFSET.
- The leading hour 1 uses the existing 1px H1_ONE_X offset.
- All 24-hour spacing, 14px stroke thickness, and outer margins remain unchanged.

## v2.0.2
- 24-hour leading digit 1 now uses the same 8px ONE_X_OFFSET as every other 1.
- 12-hour leading-digit geometry remains unchanged.

## v2.0.2
- Moved Time Format to the first setting under Appearance.

## v2.0.2
- Added Center 12 Hour Clock.
- When enabled, 1:00-9:59 centers the visible H:MM group.
- 10:00-12:59 keeps the original four-digit spacing.

## v2.0.2
- Disabled Center 12 Hour Clock in settings whenever 24 Hour Time Format is selected; it re-enables when returning to 12 Hour.
- All four 24-hour digit cells now use the same 39px width.
- Removed the special first-digit geometry/inset in 24-hour mode.
- Kept the 24-hour clock at the previous 189px total width with 14px strokes and existing digit/colon spacing.

## v2.0.2
- Bottom-bar Day of Week, Date, and Month now match the header calendar style.
- Calendar items use the large custom header font with no small label.
- Calendar values are centered horizontally and vertically in left, center, or right footer slots.
- Other footer item layouts are unchanged.

## v2.0.2
- Added left/right-only bar choices for Calories, Distance, Sunrise, Sunset, and High / Low Temp.
- Calories use Pebble Health active kilocalories for the current day.
- Distance uses Pebble Health walked distance; miles are shown with Fahrenheit and kilometers with Celsius.
- Sunrise/Sunset follow the selected 12/24-hour time format.
- High/Low uses the current temperature plus today's available 3-hour OpenWeather forecast points.
- Top and bottom center choices remain unchanged.

## v2.0.2
- Removed Day of Week and Month from top-center and bottom-center choices.
- Center Weather now displays TEMP as the label with a centered temperature value and no weather icon.
- Moved the top-center battery icon upward and matched its value layout to the bottom-center battery.

## v2.0.2
- Step progress bar now switches to the Accent Color when the daily step goal is reached.

## v2.0.2
- Added optional Raise to Wake modes: Off, Normal, and Sensitive.
- Uses 10 Hz accelerometer samples batched five at a time (two app callbacks per second).
- Detects a lowered/edge-on wrist followed by a stable face-toward-user orientation within a short gesture window.
- Ignores vibration-contaminated samples, rejects high-impact samples, and uses a cooldown to reduce false wakes.
- Triggers Pebble's system-managed short backlight with light_enable_interaction().

## v2.0.2
- Reworked Raise to Wake around an upright, wearer-facing read pose instead of a face-up pose.
- The detector no longer requires the wrist to begin from a hanging/down position.
- Any meaningful recent wrist motion can arm the detector; entering and briefly stabilizing in the read pose triggers the backlight.
- Uses Y-dominant / low-Z orientation thresholds, motion memory, stable-pose confirmation, and cooldown filtering.
- Raise logs now include x/y/z values for tuning on real hardware.

## v2.0.2
- Raise to Wake now preserves the Y-axis sign instead of using |Y|.
- Wearer-facing orientation requires negative Y, based on PT2 hardware logs.
- Mirror-image positive-Y orientation (watch flipped away) is rejected.

## v2.0.2
- Normal Raise to Wake now requires slightly more wrist movement (motion threshold 180 -> 210).
- Normal motion-memory window shortened from 1100 ms to 950 ms to reduce incidental triggers.
- Accelerometer remains at 10 Hz, but batching increased from 5 to 10 samples.
- App-side accelerometer callbacks drop from about 2 per second to about 1 per second, reducing CPU wakeups while retaining the same sensor sampling fidelity.
- Sensitive mode thresholds remain unchanged.

## v2.0.2
- Decoupled conditional UI visibility from the physical backlight LED.
- Successful Raise to Wake starts a 4-second logical interaction window even if ambient light suppresses the LED.
- PT2 screen Touchdown also starts the logical interaction window and requests the normal system backlight interaction.
- Top Bar, Bottom Bar, and backlight-only Step Bar now show whenever either the real backlight is on OR the logical interaction window is active.
- Added raise logs showing both logical interaction and physical light state for outdoor testing.

## v2.0.2 — Runtime Free / Pro licensing foundation
- Converted the project from separate Standard/Pro compile-time editions to one runtime-gated build.
- Free users: 5000-step target, Time Format (12/24 hour), and Center 12 Hour Clock only.
- Center 12 Hour Clock defaults ON for free users.
- All colors, weather units, bar customization, step-goal changes, progress layouts, and Raise to Wake require Pro.
- Pro restrictions are enforced in C so manually-crafted AppMessages cannot bypass the settings UI.
- Reserved PRO_LICENSE=24 and LICENSE_CHECK=25 for the KiezelPay license bridge.

## v2.0.2
- Forced Clay's full COLOR palette for Accent, Clock, and Background controls.
- Prevents PT2/Emery configuration from falling back to the black/white palette.

## v2.0.2
- Added missing Pebble package dependencies required by the KiezelPay integration:
  - kiezelpay-core ^2.0.3
  - pebble-events ^1.0.2
- Keeps the forced full COLOR Clay palette from v2.0.1.
