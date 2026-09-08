# Big Time — Pebble Time 2 Watchface

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


## v2.5.0 clock auto-contrast refinement

When the clock color is pure white or pure black, changing the background color now automatically switches the clock between white and black using Pebble's legibility helper. Any non-neutral custom clock color remains unchanged.


## v2.5.0
Top and bottom bars now share customizable content choices. Day of Week, Date, and Month are available in bottom slots, while Weather, Steps, Battery, Heart Rate, Bluetooth, Day of Week, Date, and Month can be assigned to any top slot.


## v2.5.0
- Added Restore Default Settings button with confirmation dialog in Clay.
- Added Always Hidden modes for both Top Bar and Bottom Bar.
- Renamed the step progress bar's Hidden option to Always Hidden for consistency.

- Replaced native JavaScript restore confirmation/alert dialogs with an in-page confirmation card and status message.

- Shortened the restore confirmation action button to Yes for compact mobile layout.

- Constrained restore confirmation buttons to equal 92px widths for reliable mobile layout.

## v2.5.0
- Added selectable 12-hour / 24-hour clock format.
- Preserved the original 12-hour digit geometry unchanged.
- Added dedicated 24-hour geometry: 110px height, 12px strokes, 36px first-hour cell, 42px remaining digit cells, with the original 6px digit/colon spacing.
- Existing settings migrate forward with 12-hour mode as the default.

## v2.5.0
- 24-hour digit 1 now uses the same full-width seven-segment cell geometry as the other numerals.
- 12-hour digit geometry is unchanged.

## v2.5.0
- Increased 24-hour digit stroke thickness from 12px to 13px.
- Narrowed the three regular 24-hour digit cells from 42px to 41px, increasing the outer margins while preserving the existing 6px digit/colon gaps and 110px clock height.

## v2.5.0
- Increased 24-hour stroke thickness from 13px to 14px.
- Narrowed regular 24-hour digit cells from 41px to 40px, increasing the outer margins to about 5.5px per side.
- Applied the original 12-hour first-digit 1px inset to the leading 24-hour digit cell while retaining its wider geometry.

## v2.5.0
- 24-hour digit 1 keeps a full-width allocated cell but now uses the same deliberate left-biased glyph placement as the 12-hour clock.
- Regular 1s use the existing 8px ONE_X_OFFSET.
- The leading hour 1 uses the existing 1px H1_ONE_X offset.
- All 24-hour spacing, 14px stroke thickness, and outer margins remain unchanged.

## v2.5.0
- 24-hour leading digit 1 now uses the same 8px ONE_X_OFFSET as every other 1.
- 12-hour leading-digit geometry remains unchanged.

## v2.5.0
- Moved Time Format to the first setting under Appearance.

## v2.5.0
- Added Center 12 Hour Clock.
- When enabled, 1:00-9:59 centers the visible H:MM group.
- 10:00-12:59 keeps the original four-digit spacing.

## v2.5.0
- Disabled Center 12 Hour Clock in settings whenever 24 Hour Time Format is selected; it re-enables when returning to 12 Hour.
- All four 24-hour digit cells now use the same 39px width.
- Removed the special first-digit geometry/inset in 24-hour mode.
- Kept the 24-hour clock at the previous 189px total width with 14px strokes and existing digit/colon spacing.

## v2.5.0
- Bottom-bar Day of Week, Date, and Month now match the header calendar style.
- Calendar items use the large custom header font with no small label.
- Calendar values are centered horizontally and vertically in left, center, or right footer slots.
- Other footer item layouts are unchanged.

## v2.5.0
- Added left/right-only bar choices for Calories, Distance, Sunrise, Sunset, and High / Low Temp.
- Calories use Pebble Health active kilocalories for the current day.
- Distance uses Pebble Health walked distance; miles are shown with Fahrenheit and kilometers with Celsius.
- Sunrise/Sunset follow the selected 12/24-hour time format.
- High/Low uses the current temperature plus today's available 3-hour OpenWeather forecast points.
- Top and bottom center choices remain unchanged.

## v2.5.0
- Removed Day of Week and Month from top-center and bottom-center choices.
- Center Weather now displays TEMP as the label with a centered temperature value and no weather icon.
- Moved the top-center battery icon upward and matched its value layout to the bottom-center battery.

## v2.5.0
- Step progress bar now switches to the Accent Color when the daily step goal is reached.

## v2.5.0
- Added optional Raise to Wake modes: Off, Normal, and Sensitive.
- Uses 10 Hz accelerometer samples batched five at a time (two app callbacks per second).
- Detects a lowered/edge-on wrist followed by a stable face-toward-user orientation within a short gesture window.
- Ignores vibration-contaminated samples, rejects high-impact samples, and uses a cooldown to reduce false wakes.
- Triggers Pebble's system-managed short backlight with light_enable_interaction().

## v2.5.0
- Reworked Raise to Wake around an upright, wearer-facing read pose instead of a face-up pose.
- The detector no longer requires the wrist to begin from a hanging/down position.
- Any meaningful recent wrist motion can arm the detector; entering and briefly stabilizing in the read pose triggers the backlight.
- Uses Y-dominant / low-Z orientation thresholds, motion memory, stable-pose confirmation, and cooldown filtering.
- Raise logs now include x/y/z values for tuning on real hardware.

## v2.5.0
- Raise to Wake now preserves the Y-axis sign instead of using |Y|.
- Wearer-facing orientation requires negative Y, based on PT2 hardware logs.
- Mirror-image positive-Y orientation (watch flipped away) is rejected.

## v2.5.0
- Normal Raise to Wake now requires slightly more wrist movement (motion threshold 180 -> 210).
- Normal motion-memory window shortened from 1100 ms to 950 ms to reduce incidental triggers.
- Accelerometer remains at 10 Hz, but batching increased from 5 to 10 samples.
- App-side accelerometer callbacks drop from about 2 per second to about 1 per second, reducing CPU wakeups while retaining the same sensor sampling fidelity.
- Sensitive mode thresholds remain unchanged.

## v2.5.0
- Decoupled conditional UI visibility from the physical backlight LED.
- Successful Raise to Wake starts a 4-second logical interaction window even if ambient light suppresses the LED.
- PT2 screen Touchdown also starts the logical interaction window and requests the normal system backlight interaction.
- Top Bar, Bottom Bar, and backlight-only Step Bar now show whenever either the real backlight is on OR the logical interaction window is active.
- Added raise logs showing both logical interaction and physical light state for outdoor testing.

## v2.5.0 — Runtime Free / Pro licensing foundation
- Converted the project from separate Standard/Pro compile-time editions to one runtime-gated build.
- Free users: 5000-step target, Time Format (12/24 hour), and Center 12 Hour Clock only.
- Center 12 Hour Clock defaults ON for free users.
- All colors, weather units, bar customization, step-goal changes, progress layouts, and Raise to Wake require Pro.
- Pro restrictions are enforced in C so manually-crafted AppMessages cannot bypass the settings UI.
- Reserved PRO_LICENSE=24 and LICENSE_CHECK=25 for the KiezelPay license bridge.

## v2.5.0
- Forced Clay's full COLOR palette for Accent, Clock, and Background controls.
- Prevents PT2/Emery configuration from falling back to the black/white palette.

## v2.5.0
- Added missing Pebble package dependencies required by the KiezelPay integration:
  - kiezelpay-core ^2.5.0
  - pebble-events ^1.0.2
- Keeps the forced full COLOR Clay palette from v2.0.1.

## v2.5.0
- Disabled Clay's automatic configuration event handling.
- Added explicit showConfiguration handler using clay.generateUrl() + Pebble.openURL().
- Added explicit webviewclosed handler using clay.getSettings() + Pebble.sendAppMessage().
- Intended to prevent settings-button failures when Clay is used alongside the KiezelPay package stack.

## v2.5.0 — Real KiezelPay entitlement integration
- Integrated the official generated KiezelPay file for product 915728151.
- Enabled KiezelPay timed-trial support so the server-configured 48-hour trial can unlock Pro.
- KIEZELPAY_TRIAL_STARTED and KIEZELPAY_LICENSED now unlock Big Time Pro.
- KIEZELPAY_TRIAL_ENDED returns the watchface to Free mode.
- KiezelPay and Big Time now share AppMessage through pebble-events rather than competing for the single native callback.
- Added the official KiezelPay JavaScript companion initialization.
- Weather JS now ignores KiezelPay AppMessages instead of treating every package message as a weather request.
- Test mode and verbose KiezelPay logging remain enabled for purchase/trial testing.

## v2.5.0 — KiezelPay auto-purchase/state fix
- Disabled KiezelPay's automatic time-trial flow again.
- Opening Settings no longer starts a trial or purchase-code sequence.
- Removed persistent phone-side Pro caching as a source of truth.
- Settings now asks the watch for authoritative license status every time it opens.
- If the watch does not confirm Pro, the settings page fails closed to Free.
- A real KiezelPay LICENSED event is the only KiezelPay event that unlocks Pro.
- The optional 48-hour Pro trial will be implemented as an explicit user-started Big Time trial, not as KiezelPay's auto-start trial.

## v2.5.0 — Explicit trial and purchase controls
- Added a visible 48-hour Pro trial notice to Free settings.
- Added Try Pro Free toggle; trial begins only after enabling it and tapping Save.
- Added Unlock Pro with KiezelPay toggle; purchase code begins only after enabling it and tapping Save.
- Added Purchase / Restore Pro control while a trial is active.
- Big Time manages the explicit 48-hour trial itself so KiezelPay's automatic timed trial remains disabled.
- Trial state persists across watchface restarts.
- KiezelPay remains the authority for purchased/restored Pro licenses.
- New AppMessage keys: TRY_PRO_FREE=26 and UNLOCK_PRO=27.

## v2.5.0
- Fixed build error in TRY_PRO_FREE and UNLOCK_PRO handlers by passing the required fallback argument to tuple_to_int32().

## v2.5.0
- Refined black/white clock-background conflict handling.
- Selecting Black Clock while Background is Black now preserves the black clock and flips Background to White.
- Selecting White Clock while Background is White now preserves the white clock and flips Background to Black.
- Changing Background to match an existing black/white clock still flips the clock for contrast.
- Custom non-black/white clock colors remain unchanged.

## v2.5.0
- Settings Save now mirrors the watch's black/white clock/background conflict logic before values are sent.
- If Black Clock + Black Background is saved, Background is stored/sent as White.
- If White Clock + White Background is saved, Background is stored/sent as Black.
- This keeps Clay's remembered settings aligned with what the watch actually displays.
- On a fresh Free -> Pro unlock, the Pro step goal initializes to 5000 instead of reviving stale cached values from older sessions.

## v2.5.0
- Black/white clock/background conflict correction now happens live inside the Clay settings page.
- Selecting Black Clock while Background is Black visibly changes Background to White in settings.
- Selecting White Clock while Background is White visibly changes Background to Black in settings.
- Changing Background into a black/black or white/white conflict visibly changes Clock to the contrasting color.
- Added CURRENT_STEP_GOAL=28 watch-to-phone telemetry.
- Pro settings now seed the Step Goal control from the watch's actual current value.
- A newly unlocked Pro watch coming from Free therefore shows 5000 rather than stale cached values such as 14000.

## v2.5.0
- Removed all automatic Clock Color / Background Color swapping.
- Clock and background colors now remain exactly as selected, even if they match.
- Removed CURRENT_STEP_GOAL; AppMessage keys remain 0-27.
- Pro Step Goal initializes to 5000 on a new Free -> Pro unlock using the existing STEP_GOAL key (10).

## v2.5.0
- Settings title is now always Big Time.
- Added entitlement text directly below the title:
  - Free
  - Free Trial — Your Big Time Pro trial is currently active.
  - Pro — Thank you for purchasing Big Time Pro!
- Reused the existing PRO_LICENSE key (24) as a 3-state entitlement value; no new AppMessage key is required.
- PRO_LICENSE values are now 0=Free, 1=Free Trial, 2=Purchased/Restored Pro.

## v2.5.0
- Free Trial status now includes the actual remaining trial time in hours and minutes.
- Reuses the existing LICENSE_CHECK key (25) in the watch-to-phone status response; no new AppMessage key is required.

## v2.5.0 — Trial reinstall protection
- Mirrors the one-time 48-hour Pro trial to PebbleKit JS localStorage.
- PebbleKit JS localStorage survives watchapp uninstall/reinstall, so reinstalling no longer creates a fresh trial on the same phone.
- TRY_PRO_FREE (26) is reused to restore the original Unix expiration timestamp to the watch; no new AppMessage key is required.
- If the original trial is still active after reinstall, the watch resumes with the original remaining time.
- If the trial has expired, a reinstall cannot start another trial.

## v2.5.0
- Removed the “— Pro” suffix from settings section headings.
- Trial/Pro entitlement remains communicated by the status text beneath the Big Time title.

## v2.5.0 — Conditional bar state-machine fix
- Reworked logical backlight interaction state around an absolute expiration timestamp.
- Top bar, bottom bar, and backlight-only step bar can no longer remain visible solely because a stale interaction boolean survived a delayed/canceled timer.
- Every visibility check, backlight transition, focus return, and input-service refresh validates the interaction deadline.
- Replaced app_timer_reschedule() with cancel + fresh one-shot timer for each touch/raise interaction.
- Added debug logs for interaction start, timeout, physical backlight transitions, and focus returns.
- Preserves the sunlight behavior: a raise/touch can still show conditional data even when ambient-light logic suppresses the physical LED.

## v2.5.0
- Fixed the v2.0.16 build error by declaring interaction_should_be_visible() before update_stepbar_layout() uses it.
- Conditional-bar state-machine changes from v2.0.16 are otherwise unchanged.

## v2.5.0 — Deterministic Peek State
- Replaced mixed physical-backlight/logical-interaction visibility with one bounded Peek state.
- Physical backlight state is never cached or used as a persistent source of truth.
- Wrist raise, Touchdown, and Backlight ON start a four-second Peek session.
- Backlight OFF ends Peek immediately when received.
- If a Backlight OFF event is missed, the independent Peek timer/deadline still expires and hides the conditional UI.
- Focus return may sample light_is_on() once to recover the case where the watchface returns while the LED is already lit; that sample only starts a bounded Peek and is never stored.
- Touch no longer redundantly calls light_enable_interaction(); Raise to Wake still requests the system backlight while Peek begins independently.

## v2.5.0 — Entitlement-safe settings + hardened Peek
- Identified the main cause of apparent permanent Full Mode: startup temporarily enforced Free defaults (bars Always Visible, Raise to Wake Off) before trial/license restoration, allowing those defaults to overwrite the user's Pro configuration.
- Added a preserved full-preference record separate from the effective Free presentation.
- Free mode can no longer save its forced bar/raise/color defaults over saved Pro preferences.
- The two settings allowed in Free (Time Format and Center 12 Hour Clock) still persist normally.
- When a trial or purchased license is restored after a process restart, the saved Pro bar modes, step-bar mode, colors, weather settings, step goal, and Raise to Wake mode are restored.
- When Pro/trial ends, the Pro preference record is retained so a later purchase can restore the user's customization.
- Peek AppTimer expiration now unconditionally hides conditional UI; the absolute deadline remains only as backup protection.
- Added Effective UI diagnostic log showing entitlement, header/footer/step modes, and Raise to Wake mode whenever entitlement refreshes.

## v2.5.0 — Peek on watchface return
- Returning to Big Time from a menu, notification, or another app now starts a fresh four-second Peek session.
- No longer depends on light_is_on() when focus returns.
- This preserves conditional top/bottom/step visibility even when ambient-light logic suppresses the physical backlight.
- Peek remains bounded by the existing deterministic timer, so this does not reintroduce permanent full mode.

## v2.5.0 — Reliable Peek on return
- Added WindowHandler.appear/disappear lifecycle callbacks.
- WindowHandler.appear is now the primary detector for Big Time becoming visible again after menus/apps/windows.
- Every appearance after the initial launch starts a fresh bounded four-second Peek.
- Initial window appearance is ignored so Big Time does not automatically show hidden data on first launch.
- Existing AppFocusService handling remains as supplemental coverage for modal notifications/system overlays.

## v2.5.0 — System Backlight Visibility
- Removed the independent Peek timer/state entirely.
- Top bar, bottom bar, and backlight-only step bar now follow Pebble's actual system backlight state for exactly the duration the backlight is on.
- No cached backlight boolean is stored; every refresh calls light_is_on() directly.
- BacklightService is used only as a redraw notification, preventing stale ON state from causing permanent full mode.
- Focus return and WindowHandler.appear both refresh visibility from light_is_on(), covering returns from menus/notifications/apps when the backlight remains active.
- Wrist Raise and touchscreen Touchdown request the normal system backlight with light_enable_interaction(); conditional UI itself has no separate timeout.
- Tradeoff: when ambient-light logic suppresses the physical backlight, backlight-only UI remains hidden because Pebble exposes no separate public 'backlight intended but suppressed' state.

## v2.5.0
- Fixed build failure caused by duplicate s_backlight_subscribed declaration.
- System-backlight visibility behavior from v2.3.0 is otherwise unchanged.

## v2.5.0 — Five-second sunlight fallback
- Normal conditional UI follows Pebble's real system backlight for the full native backlight duration.
- Added one isolated five-second fallback only when an explicit interaction occurs while light_is_on() remains false.
- Wrist Raise, screen Touchdown, focus return, and watchface window appearance request the normal backlight and use the fallback only if the LED stays off.
- Backlight ON immediately cancels the fallback, so normal operation has no synthetic timeout.
- Repeated fallback interactions refresh one timer; timers never stack.
- No physical backlight state is cached, preventing stale ON state from causing permanent full mode.

## v2.5.0 — One-time trial enforcement
- Fixed expired-trial logic that deleted the watch's trial record and unintentionally made the trial appear unused again.
- Expired trials now retain a permanent used marker on the watch.
- Phone localStorage now restores either the original active expiry or an explicit used/expired marker after reinstall.
- The watch rejects new trial-start requests once the used marker exists.
- Settings receives the authoritative used-trial state and removes the Try Pro Free toggle after the trial has been consumed.
- Expired users now see "Your 48-hour Pro trial has ended." while the KiezelPay purchase/restore option remains available.

## v2.5.0 — Preserve trial configuration through purchase
- Audited Free Trial -> Free -> Purchased Pro preference persistence.
- Fixed trial-expiry transition so an active trial immediately switches to Free when no purchased KiezelPay license exists.
- That transition now reliably captures the complete final trial configuration before Free defaults are applied.
- A later KiezelPay purchase restores the preserved Pro configuration.
- Purchasing while the trial is still active leaves the current Pro configuration intact and changes entitlement to Purchased Pro without resetting settings.
- Free-mode changes to Time Format and Center 12 Hour Clock continue updating those two fields in the preserved preference record without overwriting premium preferences.

## v2.5.0 — Purchased Pro entitlement synchronization
- Fixed Settings-open race where sessionLicenseKnown could remain true from an older Free response and allow Clay to open before a fresh purchased entitlement arrived.
- Every Settings open now requires a fresh watch response before using entitlement state.
- Increased the Settings response fallback from 900 ms to 1500 ms and fails closed without reusing stale entitlement.
- Added retry-capable watch-to-phone license status messages to handle AppMessage contention while KiezelPay is completing a purchase.
- KIEZELPAY_LICENSED immediately enables Pro locally and schedules a delayed/retried status sync to the phone.
- Added entitlement/status diagnostic logs for purchase testing.

## v2.5.0
- Fixed build error by forward-declaring schedule_license_status_retry() before the KiezelPay event callback uses it.
- Purchased-Pro entitlement synchronization logic from v2.4.3 is otherwise unchanged.

## v2.5.0 — Synchronize Big Time with validated KiezelPay status
- Fixed the core purchase/restore mismatch exposed by logs: KiezelPay returned licensed while Big Time had already answered Clay with Free.
- Big Time no longer immediately reports Free while KiezelPay's asynchronous status check is unresolved.
- Watches the KiezelPay v2.2.4 status-result tuple after kiezelpay-core's earlier pebble-events handler has processed the same dictionary.
- KIEZELPAY_STATUS_RESULT=2 now synchronizes Big Time to Purchased Pro, restores preserved Pro settings, and sends PRO_LICENSE=2 to the phone.
- Non-licensed KiezelPay results return Big Time to Free only when no Big Time trial is active.
- KIEZELPAY_CODE_AVAILABLE also marks the KiezelPay state as known/unlicensed.
- Settings waits up to four seconds for the fresh KiezelPay/watch entitlement instead of opening immediately from stale Free state.
- An already-purchased user does not need another code: a licensed KiezelPay status automatically restores Pro.

## v2.5.0 — Faster Purchased Pro settings
- Purchased Pro no longer waits for a fresh KiezelPay/watch round-trip every time Settings is tapped.
- A confirmed Purchased Pro UI state is cached in PebbleKit JS localStorage and opens the Pro Clay page immediately.
- The watch still performs a background entitlement refresh after opening.
- C-side entitlement remains authoritative, so cached UI cannot unlock premium settings by itself.
- An authoritative Free response clears the purchased UI cache.
- Removed the KiezelPay Purchase / Restore section from Purchased Pro settings.
- The KiezelPay section remains visible during the Free Trial so trial users can purchase before expiry.

## v2.5.0 — Production release candidate
- Disabled KiezelPay test mode.
- Disabled verbose KiezelPay logging.
- No other watchface behavior changed from v2.4.6.
- Intended for final real-purchase, persistence, restore, and reinstall testing before Pebble Store deployment.

## v2.5.1 — Big Time product naming
- Updated Pebble app/watchface display metadata to Big Time.
- Replaced remaining legacy product-name references in project text/code.
- Production KiezelPay flags remain unchanged from v2.5.0.

## v2.5.2 — Keyless weather
- Removed the embedded OpenWeatherMap API key and all OpenWeatherMap requests.
- Weather now follows Pebble's current C-watchface pattern: PebbleKit JS obtains the phone location, requests Open-Meteo, and sends the result to the watch through AppMessage.
- Open-Meteo requires no API key or embedded secret.
- Current temperature, weather icon category, today's high/low, sunrise, and sunset continue using the existing Big Time AppMessage keys.
- Sunrise/sunset are requested with timezone=auto and parsed as local time.

## v2.5.3 — Purchased Pro persistence hotfix
- A positively validated KiezelPay purchase now creates a dedicated persistent Purchased Pro marker on the watch.
- Purchased Pro is restored immediately at watchface startup before network/Bluetooth/KiezelPay refreshes complete.
- KIEZELPAY_LICENSED and validated KIEZELPAY_STATUS_RESULT=2 both persist Purchased Pro.
- Network errors, Bluetooth errors, delayed checks, restarts, and non-final KiezelPay statuses do not revoke purchased Pro.
- Only an explicit validated unlicensed result (status 0 / purchase-code-required state) clears the Purchased Pro marker.
- Trial expiration can no longer demote a persisted purchased user.
- Existing Pro preferences are restored immediately with the purchased entitlement.

## v2.6.1 — Optional data labels (build fix)
- Adds the Pro-only Show Data Labels setting.
- Hiding labels enlarges Weather/Temp, HR, Steps, Calories, Distance, Sunrise, Sunset, and High/Low values to the same font used by Day / Date / Month.
- Hidden-label values are vertically and horizontally centered in their slot.
- Existing users migrate with labels enabled.
- Corrected the v2.6.0 settings migration so historical WatchfaceSettings typedefs are declared only once.

## v2.6.2
- Fixed build error in optional-label weather icon visibility handling.
- Converted BitmapLayer pointers to Layer pointers with bitmap_layer_get_layer() before calling layer_set_hidden().
- Optional data-label behavior and v13-to-v14 settings migration are unchanged from v2.6.1.

## v2.6.3 — Side alignment refinements
- Label-hidden metric values in left-side slots are now left-aligned.
- Label-hidden metric values in right-side slots are now right-aligned.
- Day / Date / Month retain their established centered treatment.
- Side weather icons are positioned at the right edge of their slot.
- With labels hidden, the weather icon is vertically centered with the enlarged temperature value.
- Weather temperature text uses the remaining space beside the icon to avoid overlap.

## v2.6.4 — Large-value vertical alignment
- Moved newly enlarged label-hidden metric values down 3 pixels for better vertical centering.
- Day / Date / Month retain their established vertical positioning.
- Weather icon vertical placement is unchanged.
- Restored weather temperature TextLayers to the full slot width so the degree symbol is no longer truncated into an ellipsis.
- Weather icon remains right-aligned and may overlap the far edge of the temperature text slightly.

## v2.6.5 — Adaptive data sizing + free temperature units
- Label-hidden Steps automatically uses the smaller value font at 10,000 steps and above.
- Label-hidden Weather temperature automatically uses the smaller value font at 100° and above (and for longer negative temperatures).
- Label-hidden High / Low temperature always uses the smaller value font so values such as 88°/72° fit reliably.
- Normal shorter values continue using the large Day / Date / Month font.
- Fahrenheit / Celsius selection is now available in the Free edition.
- Free-mode persistence now preserves Temperature Unit alongside Time Format and Center 12 Hour Clock.

## v2.6.6 — Medium adaptive data font
- Added a 28px bold medium data font between the 31px large calendar font and 24px standard value font.
- Three-digit temperatures use Medium when labels are hidden.
- Steps at 10,000 and above use Medium when labels are hidden.
- High / Low temperature always uses Medium when labels are hidden.
- Distance always uses Medium when labels are hidden.
- Shorter data values continue using the large Day / Date / Month font.

## v2.6.7 — Center and solar refinements
- Removed Steps from both Top Center and Bottom Center selectors.
- Existing center-Steps layouts fall back to Weather (top) or Heart Rate (bottom).
- Right-side Weather icon moved to the left edge of the right slot while temperature remains right-aligned.
- Center Weather temperature nudged 2 pixels right for better optical centering.
- Sunrise and Sunset include AM/PM in 12-hour mode and use Medium when labels are hidden.
- Distance currently follows the selected temperature system: Fahrenheit = MI and Celsius = KM; there is no separate native watch distance-unit lookup in this version.

## v2.7.0 — Per-slot label controls
- Replaced the single global Show Data Labels setting with independent Hide Label controls for all six top/bottom bar slots.
- Each Hide Label toggle appears only when its selected data item actually has a descriptive label.
- Hiding a label affects only that one slot and keeps the existing enlarged/adaptive font behavior.
- Existing v2.6.x users migrate automatically: the former global hidden-label choice is applied to all six slots.
- Calendar, battery, and Bluetooth selections do not show a Hide Label toggle because they do not use the same descriptive-label treatment.

## v2.7.1 — Default layout consistency
- Fresh-install defaults explicitly show all six data labels.
- Bottom Right defaults to Steps.
- Restore Default Settings now resets every per-slot Hide Label toggle to off.
- Restore Default Settings explicitly restores Bottom Right to Steps.
- Free defaults, fresh-install defaults, Clay defaults, and manual Restore Defaults now use the same baseline.

## v2.7.2 — Restore Steps side slots
- Restored Steps to the shared side-slot list used by Top Left, Top Right, Bottom Left, and Bottom Right.
- Removed the lingering Steps option from Top Center.
- Steps remains unavailable in both center positions as intended.
- Bottom Right continues to default to Steps.

## v2.8.0 — Watchface translations
- Added a Language selector to settings with English and Swedish (Svenska).
- Swedish translates watchface data labels plus day-of-week and month abbreviations.
- Language is a Free setting and persists independently of Pro entitlement.
- Added a table-driven translation layer in main.c so future languages can be added by supplying one translation record and one settings option.
- Restore Defaults returns the watchface language to English.

## v2.8.1 — Persistent weather cache
- Weather remains visible when Big Time returns from menus or the watchface process restarts.
- The watch persists the last successful temperature, icon, high/low, sunrise, and sunset values and restores them before drawing the UI.
- PebbleKit JS also caches the last successful Open-Meteo payload in localStorage.
- Cached weather remains static until the next scheduled refresh.
- Open-Meteo/location refreshes now run on a 30-minute interval instead of fetching every PebbleKit launch.
- Failed refreshes leave the previous weather visible.
- Refresh timing is centralized in WEATHER_REFRESH_MS for easy future configuration.

## v2.8.2 — Configurable weather refresh interval
- Added Weather Refresh Interval to Big Time settings.
- Available intervals: 30 minutes, 1 hour, 2 hours, 3 hours, and 6 hours.
- Default interval is 1 hour.
- The selected interval is stored on the phone side and survives watchface restarts.
- Cached weather remains displayed continuously between refreshes.
- Changing the interval does not clear or immediately replace cached weather; it recalculates when the next refresh is due.
- Failed location/network/Open-Meteo refreshes continue showing the last successful cached weather.
- Weather Refresh is intentionally phone-side only and is not sent through AppMessage to the watch.

## v2.8.3 — Language selection fix
- Fixed Swedish/English selection being overwritten by Free-mode default enforcement.
- Language now persists correctly for both Free and Pro users.
- Free-mode enforcement explicitly preserves Time Format, Center 12 Hour Clock, Temperature Unit, and Language.
- Added watch-side logging for language changes to simplify future translation testing.

## v2.8.4 — Language build fix
- Moved the WatchLanguage enum alongside the other settings enums so LANG_ENGLISH and LANG_SWEDISH are declared before Free-default logic uses them.
- No translation behavior changed from v2.8.3.
- Swedish/English selection still persists for both Free and Pro users.

## v2.8.5 — Swedish calendar text fix
- Expanded weekday and month buffers for UTF-8 translations.
- Fixes MÅN being truncated to MÅ because Å uses two bytes in UTF-8.
- Adds headroom for future translated calendar abbreviations.

## v2.9.0 — Expanded watchface translations
- Added Spanish, French, German, and Portuguese.
- English and Swedish remain available.
- Translates watchface labels, weekday abbreviations, and month abbreviations.
- Settings interface remains English.
- Languages remain centralized in the WatchTranslation table for easy expansion.

## v2.9.1 — Cached temperature display fix
- Fixed cached temperature briefly appearing blank when Big Time returns from Settings or menus.
- The watch was already restoring the raw cached temperature, but the formatted temperature string was not rebuilt until another AppMessage arrived.
- Cached temperature text is now formatted immediately after weather cache restoration, before the watchface UI appears.
- Temperature, weather icon, high/low, sunrise, and sunset continue to use the same cached weather snapshot and refresh interval.

## v2.9.2 — Free languages and atomic cached weather
- All six watchface languages are explicitly available and persisted for Free users: English, Swedish, Spanish, French, German, and Portuguese.
- Language selection is now also persisted phone-side and re-seeded into Clay whenever Settings opens, independent of Pro entitlement.
- Fixed the remaining temperature pop-in: window_load was resetting the already-restored cached temperature display string to "--".
- The window now preserves the formatted cached temperature prepared during init, so temperature appears with the rest of the cached weather immediately.
- Temperature, icon, high/low, sunrise, and sunset continue to update together on the selected weather refresh interval.

## v2.9.3 — Language selector cache fix
- Fixed the Settings language dropdown sometimes showing only English and Swedish after upgrading.
- Big Time now explicitly injects all six supported language options into the live Clay configuration every time Settings opens.
- This overrides stale cached configuration objects on the Pebble companion.
- All six languages remain Free: English, Swedish, Spanish, French, German, and Portuguese.

## v2.9.4 — Settings language synchronization
- The watch is now the authoritative source for the currently selected watchface language.
- Current LANGUAGE is included in the existing watch-to-phone license/status response.
- PebbleKit JS stores the language reported by the watch before opening Settings.
- The Language selector therefore opens on the language the watch is actually displaying instead of falling back to stale phone-side/default English.
- No new AppMessage key is required; the existing LANGUAGE key is reused bidirectionally.

## v2.9.5 — Development preview build
- KiezelPay TEST MODE enabled for on-watch Pro purchase/entitlement testing without a production payment.
- KiezelPay verbose logging enabled for final validation.
- This build is for development preview only and must not be published.
- Before release, KIEZELPAY_TEST_MODE and KIEZELPAY_LOG_VERBOSE must both be returned to 0.

## v2.9.6 DEV — 15-minute weather refresh
- Added 15 Minutes as a Weather Refresh Interval option.
- Existing 30-minute, 1-hour, 2-hour, 3-hour, and 6-hour options remain unchanged.
- Cached weather continues to remain visible between scheduled refreshes.
- Default refresh interval remains 1 hour.
- Built from the stable pre-Time-Zone v2.9.5 DEV baseline.

## v2.9.7 DEV — Top hidden-label vertical alignment
- Vertically centered enlarged values in the top bar when Hide Label is enabled.
- Applies consistently to top-left, top-center, and top-right.
- Normal labeled layouts and calendar positioning are unchanged.
- Retains the 15-minute Weather Refresh option from v2.9.6.

## v2.9.8 DEV — Language selector synchronization
- Settings now keeps a session copy of the language reported by the watch.
- When Settings opens, the Language selector prefers the watch-authoritative value over stale phone storage.
- Saving a new language updates both session state and persistent phone storage immediately.
- Existing language choices and translations are unchanged.
- Retains the 15-minute weather option and top hidden-label alignment fixes.

## v2.9.9 DEV — Correct top hidden-label vertical centering
- Corrected the previous hidden-label alignment adjustment, which moved top values in the wrong direction.
- Hidden-label values in Top Left, Top Center, and Top Right are moved up 6 px from v2.9.8.
- Their top edge now matches the y=4 geometry already used by the large calendar-style text in the same 52 px header.
- Normal labeled layouts are unchanged.

## v2.9.10 DEV — Language save race fix
- Fixed a race where the watch could successfully switch languages but an older in-flight status response would temporarily reset the Settings selector to the previous language.
- A newly saved language now remains authoritative until the watch reports that same language back.
- Stale mismatched watch-language responses are ignored while confirmation is pending.
- Settings prioritizes the just-saved language, then the latest confirmed watch language, then phone storage.
- Existing translations, weather refresh options, and top hidden-label alignment are unchanged.

## v2.9.11 DEV — Correct language before Settings opens
- Fixed the remaining Language selector timing issue for purchased Pro users.
- Purchased Settings no longer opens immediately from cached entitlement before the watch replies.
- Big Time now requests a fresh LICENSE_CHECK/status response first; that response already contains the watch's current LANGUAGE.
- The existing AppMessage language handler updates the selected language before the entitlement handler opens Clay.
- Added a 1.5-second fallback so Settings cannot become unresponsive if the status response is delayed.
- This directly fixes the observed sequence where Settings opened as English and Swedish arrived immediately afterward.

## v2.9.12 DEV — Battery display variants
- Added Battery Icon and Battery % while preserving Battery (icon + %).
- Battery Icon is icon-only and uses a larger, thicker 42x16 battery graphic.
- Battery % is percentage-only, with no icon or label, using the enlarged value layout.
- New variants are available in all top and bottom positions, including centers.
- Existing combined battery behavior remains unchanged.

## v2.9.13 DEV — Top battery alignment
- Moved the combined Battery (icon + %) icon upward in the top-left and top-right slots.
- Top combined battery formatting now matches the bottom-bar layout, with the icon above the percentage instead of overlapping it.
- Top-center combined battery was already correctly positioned and remains unchanged.

## v2.9.14 DEV — Battery center validation and icon alignment
- Fixed new Battery Icon / Battery % choices in Top Center being accepted by the AppMessage handler but rejected by settings validation.
- Top Center and Bottom Center validation now mirror their actual allowed selector values.
- Prevents the safety validator from restoring defaults when a new center battery option is selected.
- Raised icon-only battery in Top Left and Top Right from y=18 to y=15, matching the bottom icon-only battery position.
- Existing combined battery and language synchronization behavior are unchanged.

## v2.9.15 DEV — Build fix
- Removed redundant >= 0 checks from uint8_t center-slot validation.
- Fixes Pebble SDK -Werror=type-limits compilation failure.
- Keeps the v2.9.14 center battery validation fix and top icon alignment unchanged.

## v2.9.16 DEV — Center battery sizing and alignment
- Raised the top-center Battery Icon from y=18 to y=15 so it matches the top-left and top-right icon-only battery alignment.
- Narrowed center Battery Icon from 42 px to 36 px in both top-center and bottom-center to better fit the smaller center areas.
- Battery % now always uses the medium font in top-center and bottom-center to prevent truncation.
- Left/right Battery Icon sizing remains unchanged.

## v2.9.17 DEV — Top Bluetooth alignment
- Raised connected Bluetooth icons in Top Left and Top Right by 3 px.
- Corrects the visually low alignment in the upper side slots.
- Top Center and bottom Bluetooth positioning remain unchanged.

## v3.0.0 DEV RC — Isolated side-slot Time Zones
- Reimplemented Time Zone support directly on the stable v2.9.17 release-candidate baseline.
- Time Zone is available only in Top Left, Top Right, Bottom Left, and Bottom Right.
- Top Center and Bottom Center are untouched.
- Selecting Time Zone reveals one slot-specific Time Zone dropdown using the existing Clay slot-change listener; no competing listener is added.
- Timezone choices are persisted separately from WatchfaceSettings, so SETTINGS_VERSION and all existing saved settings remain untouched.
- Local Time uses the watch's native local clock/timezone.
- Additional choices are explicit UTC offsets from UTC-12:00 through UTC+14:00, including common half-hour and quarter-hour offsets.
- Time Zone values follow Big Time's current 12/24-hour format.
- Hide Label is supported for Time Zone; the enlarged timezone time uses the medium font for reliable fit.
- No changes were made to KiezelPay entitlement, language synchronization, weather caching, battery options, or center-slot behavior.

## v3.0.1 DEV RC — Time Zone AppMessage capacity fix
- Increased the watch AppMessage inbox from 256 bytes to 512 bytes.
- Time Zone adds four values to Clay's complete Save Settings payload; the previous 256-byte inbox could reject the enlarged dictionary before any setting reached the watch.
- Outbox remains 256 bytes.
- No Time Zone logic, existing slot logic, licensing, language, weather, or layout behavior was otherwise changed.

## v3.0.2 DEV RC — Dynamic Time Zone labels
- Time Zone slot labels now show the actual selected zone instead of the generic TZ label.
- Examples: UTC+02:00, UTC-05:00, UTC+05:30, or LOCAL.
- Each of the four side slots uses its own selected timezone label independently.
- Hide Label behavior is unchanged.
- No changes to AppMessage sizing, timezone persistence, licensing, or existing slot behavior.

## v3.0.3 DEV RC — AM/PM for Time Zone values
- Time Zone values now include AM/PM when Big Time is using 12-hour time.
- Example: 2:35 PM.
- 24-hour Time Zone values remain unchanged, e.g. 14:35.
- Dynamic timezone labels and all prior RC fixes are unchanged.

## v3.0.4 DEV RC — Timezone label build fix
- Added the missing forward declaration for top_slot_label().
- Fixes the GCC implicit-declaration/conflicting-types build failure in top_side_slot_label().
- No runtime behavior or existing timezone logic was changed.

## v3.0.5 DEV RC — Hidden Time Zone fit
- Hidden-label Time Zone values now use the smaller standard value font instead of the medium hidden-label font.
- Prevents 12-hour timezone strings such as 12:59 PM from being truncated with an ellipsis.
- Applies to Top Left, Top Right, Bottom Left, and Bottom Right only when Time Zone's label is hidden.
- Normal labeled Time Zone display and all other hidden-label items are unchanged.

## v3.0.6 DEV RC — Time value fit and AM/PM spacing
- Hidden-label Time Zone values now receive 8 px of additional text width in all four side positions.
- Allows long 12-hour strings such as 12:59 PM to extend slightly beyond the normal slot boundary instead of ellipsizing.
- Standardized time displays that include AM/PM to use a space before the suffix, e.g. 6:42 AM.
- Other slot geometry remains unchanged.

## v3.0.7 DEV RC — Robust hidden Time Zone fit
- Added a dedicated 20 px bold font for hidden-label Time Zone values.
- Expanded hidden Time Zone text frames by 16 px total so long 12-hour values such as 12:59 PM fit without ellipsis.
- Other hidden-label values and normal Time Zone labels remain unchanged.

## v3.0.8 DEV RC — Hidden Time Zone font build fix
- Replaced unsupported FONT_KEY_GOTHIC_20_BOLD with Pebble's supported FONT_KEY_GOTHIC_18_BOLD.
- Keeps the dedicated smaller hidden Time Zone font and widened text frames from v3.0.7.
- Fixes the Emery compile failure without changing Time Zone behavior.

## v3.0.9 DEV RC — Simplified Time Zone display
- Removed AM/PM from Time Zone values so side-slot times fit reliably.
- 12-hour Time Zone example: 2:35.
- 24-hour Time Zone example: 14:35.
- Reverted the dedicated hidden Time Zone font and timezone-specific widened frames.
- Hidden-label Time Zone values now use the normal medium hidden-value font and normal medium-value alignment.
- Other AM/PM displays remain unchanged.

## v3.1.0 — Production release
- Production KiezelPay mode enabled: test purchasing disabled and verbose native logging disabled.
- Adds 15-minute weather refresh.
- Adds improved hidden-label alignment and language settings synchronization.
- Adds Battery Icon and Battery % display variants.
- Adds side-slot Time Zone support for Top Left, Top Right, Bottom Left, and Bottom Right.
- Time Zone uses separate persistence keys and does not alter WatchfaceSettings or SETTINGS_VERSION.
- AppMessage inbox remains 512 bytes to safely receive the expanded Clay settings payload.
- Existing app UUID is preserved for in-place upgrades and settings continuity.

## v3.2.0 DEV — Separate hour and minute colors
- DEVELOPMENT BUILD: KiezelPay test mode and verbose logging enabled.
- Added opt-in Separate Hour / Minute Colors.
- Existing Clock Color remains the backward-compatible default and controls the whole clock when split colors are disabled.
- With split colors enabled: Hour Color controls hour digits, Minute Color controls minute digits, and Clock Color controls the colon.
- Split-color state and colors use separate persistence keys; WatchfaceSettings and SETTINGS_VERSION remain unchanged.
- Existing user customizations therefore remain binary-compatible.

## v3.2.1 DEV — Seconds options
- Added Flashing Colon: the main clock colon alternates visible/hidden once per second.
- Added Seconds as a label-free large 00–59 value in all six information positions.
- Seconds is available in Top Left, Top Center, Top Right, Bottom Left, Bottom Center, and Bottom Right.
- SECOND_UNIT ticks are used only while Flashing Colon or a Seconds slot is active; otherwise the original MINUTE_UNIT cadence is retained.
- Flashing Colon is persisted separately, preserving WatchfaceSettings v16 compatibility.
- KiezelPay development/test mode remains enabled.

## v3.2.2 DEV — Rounded Time style
- Renamed Flashing Colon to Seconds Colon in Settings.
- Added opt-in Rounded Time style.
- Rounded Time uses Pebble's native rounded-rectangle drawing for all seven-segment digit strokes and colon dots.
- Existing square segment style remains the default.
- Rounded Time is persisted separately; WatchfaceSettings v16 remains unchanged.
- DEV/KiezelPay test mode remains enabled.

## v3.2.3 DEV — Time Style dropdown
- Replaced the Rounded Time toggle with a Time Style dropdown.
- Square: original hard-corner seven-segment clock.
- Rounded: fully rounded/capsule-style segments from v3.2.2.
- Slightly Rounded: squared segments with a subtle 2 px corner radius.
- Existing v3.2.2 Rounded Time persistence is compatible: old false=Square and true=Rounded.
- WatchfaceSettings v16 remains unchanged and DEV/KiezelPay test mode remains enabled.

## v3.2.4 DEV — Slightly Rounded refinement
- Increased Slightly Rounded clock segment corner radius from 2 px to 3 px.
- Slightly Rounded colon dots now use the matching 3 px corner radius.
- Square and Rounded styles are unchanged.

## 1.3-dev-001 — Forecast + Rain Chance
- Added Forecast to Top/Bottom Left and Right: tomorrow's condition icon plus high/low.
- Added Rain Chance to all six information positions with optional RAIN label.
- Rain Chance uses today's daily precipitation_probability_max from Open-Meteo.
- Weather requests now retrieve two forecast days in the existing request and cache all values together phone-side.
- New forecast/rain watch data is persisted in a separate extended cache so the existing current-weather cache remains upgrade-compatible.
- Existing slot IDs were not renumbered; new IDs are appended.

## 1.3-dev-002 — Battery Progress Bar
- Added Track Battery Instead of Steps to the Step Progress Bar section.
- Battery uses the existing progress styles and drains from full at 100% to empty at 0%.
- Step Goal is disabled while battery tracking is selected, without deleting the saved goal.
- The new preference is persisted separately; WatchfaceSettings v16 and package version 1.3.0 remain unchanged.

## Big Time 1.3.0 — Production Release
- Production KiezelPay mode enabled.
- Adds separate hour/minute colors and three Time Styles.
- Adds Seconds Colon and Seconds information slots.
- Adds tomorrow Forecast and daily Rain Chance.
- Adds Battery as an alternate Progress Bar source.
- Preserves WatchfaceSettings v16 and existing user customization compatibility.

## 1.3-dev-004 — Tomorrow refinement
- Renamed Forecast to Tomorrow.
- Tomorrow now shows the condition icon plus tomorrow's high temperature only.
- Tomorrow's low remains cached but is not displayed.
- Hidden-label Tomorrow uses the normal large-value treatment.

## 1.3-dev-005 — +2 Hours Forecast
- Added +2 Hours to all four left/right information slots.
- Displays the forecast weather icon and temperature two hours ahead.
- Uses the same Open-Meteo hourly response and normal weather refresh/cache path.
- +2 Hours has a hideable label.
- Existing slot/message IDs remain unchanged; new IDs are appended.

## 1.3-dev-006 — Forecast cache fix + Quiet Time
- Renamed selector items to Forecast Tomorrow and Forecast +2 Hours.
- Fixed upgrade behavior where an older fresh weather cache could leave +2 Hours at `--`.
- Raise to Wake now respects Pebble system Quiet Time.

## 1.3-dev-007 — Forecast icon first-frame cache
- Forecast Tomorrow and Forecast +2 Hours icons now restore directly from watch persistence on first frame.
- Eliminates forecast-icon pop-in when returning from menus/settings.
- Weather refresh timing is unchanged; cached icons remain visible until replaced by a successful refresh.

## 1.3-dev-008 — Labeled weather icon margin
- Added 3px more outer margin to labeled weather/forecast icons.
- Hidden-label icon positioning is unchanged.

## 1.3-dev-009 — Backlight/layout stability
- Removed unnecessary clock relayout/redraw when revealing backlight-only top/bottom bars.
- Stepbar layout now redraws the clock only when its frame actually changes.
- Intended to eliminate hitch/crash behavior after switching Time Style and then tapping to reveal hidden bars.

## 1.3-dev-010 — Settings/backlight race fix
- Removed automatic backlight requests when returning from Settings or menus.
- Window/focus callbacks now only synchronize conditional bar visibility to the real backlight state.
- Touch and Raise to Wake remain the only custom paths that request the backlight.
- Fix targets crashes/hitches after any Settings change with backlight-only top/bottom bars.

## 1.3-dev-011 — Repeat settings save stability
- Makes full Clay configuration dictionaries idempotent.
- Prevents unchanged settings from repeatedly triggering persistence, layout, service, and redraw work.
- Targets the reproducible first-save-works / second-save-hangs failure.

## 1.3-dev-012 — Bluetooth Colon
- Added optional Bluetooth Colon connectivity indicator.
- Bottom colon dot is filled while connected and outlined while disconnected.
- Indicator follows Square, Slightly Rounded, Rounded, and Seconds Colon behavior.
- Repeat-save stability safeguards from dev-011 are retained.

## 1.3-dev-013 — +2 forecast icon lifecycle
- Current-weather icon updates no longer destroy forecast bitmaps owned by Tomorrow/+2 layers.
- Background changes re-tint all weather icon families independently.
- Fixes Forecast +2 Hours icon pop-in after returning from menus.


## v1.4.7 — Italian and Dutch watchface languages
- Added Italian (Italiano) and Dutch (Nederlands) display translations.
- Translates watchface labels, weekday abbreviations, and month abbreviations.
- Settings interface remains in English; only the existing language selector gains the two new choices.
- Existing language IDs and saved preferences remain unchanged.
