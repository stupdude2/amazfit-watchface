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


## v1.7.1 clock auto-contrast refinement

When the clock color is pure white or pure black, changing the background color now automatically switches the clock between white and black using Pebble's legibility helper. Any non-neutral custom clock color remains unchanged.


## v1.7.1
Top and bottom bars now share customizable content choices. Day of Week, Date, and Month are available in bottom slots, while Weather, Steps, Battery, Heart Rate, Bluetooth, Day of Week, Date, and Month can be assigned to any top slot.


## v1.7.1
- Added Restore Default Settings button with confirmation dialog in Clay.
- Added Always Hidden modes for both Top Bar and Bottom Bar.
- Renamed the step progress bar's Hidden option to Always Hidden for consistency.

- Replaced native JavaScript restore confirmation/alert dialogs with an in-page confirmation card and status message.

- Shortened the restore confirmation action button to Yes for compact mobile layout.

- Constrained restore confirmation buttons to equal 92px widths for reliable mobile layout.

## v1.7.1
- Added selectable 12-hour / 24-hour clock format.
- Preserved the original 12-hour digit geometry unchanged.
- Added dedicated 24-hour geometry: 110px height, 12px strokes, 36px first-hour cell, 42px remaining digit cells, with the original 6px digit/colon spacing.
- Existing settings migrate forward with 12-hour mode as the default.

## v1.7.1
- 24-hour digit 1 now uses the same full-width seven-segment cell geometry as the other numerals.
- 12-hour digit geometry is unchanged.

## v1.7.1
- Increased 24-hour digit stroke thickness from 12px to 13px.
- Narrowed the three regular 24-hour digit cells from 42px to 41px, increasing the outer margins while preserving the existing 6px digit/colon gaps and 110px clock height.

## v1.7.1
- Increased 24-hour stroke thickness from 13px to 14px.
- Narrowed regular 24-hour digit cells from 41px to 40px, increasing the outer margins to about 5.5px per side.
- Applied the original 12-hour first-digit 1px inset to the leading 24-hour digit cell while retaining its wider geometry.

## v1.7.1
- 24-hour digit 1 keeps a full-width allocated cell but now uses the same deliberate left-biased glyph placement as the 12-hour clock.
- Regular 1s use the existing 8px ONE_X_OFFSET.
- The leading hour 1 uses the existing 1px H1_ONE_X offset.
- All 24-hour spacing, 14px stroke thickness, and outer margins remain unchanged.

## v1.7.1
- 24-hour leading digit 1 now uses the same 8px ONE_X_OFFSET as every other 1.
- 12-hour leading-digit geometry remains unchanged.

## v1.7.1
- Moved Time Format to the first setting under Appearance.

## v1.7.1
- Added Center 12 Hour Clock.
- When enabled, 1:00-9:59 centers the visible H:MM group.
- 10:00-12:59 keeps the original four-digit spacing.

## v1.7.1
- Disabled Center 12 Hour Clock in settings whenever 24 Hour Time Format is selected; it re-enables when returning to 12 Hour.
- All four 24-hour digit cells now use the same 39px width.
- Removed the special first-digit geometry/inset in 24-hour mode.
- Kept the 24-hour clock at the previous 189px total width with 14px strokes and existing digit/colon spacing.
