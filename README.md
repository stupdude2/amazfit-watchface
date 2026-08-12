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


### v1.2.7 layout refinement
- Backlight-only step bars reserve their layout space so the clock does not move when the light changes.
- Fully hidden step bar expands the clock area and biases the time 3 px upward.
