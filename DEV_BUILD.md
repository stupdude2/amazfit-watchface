# Big Time 1.3.1 Analog Development Build

Pebble Store/package version: 1.3.1
Baseline: big-time-v1.3.1-pebble-store-release

This development build adds the first rectangular analog clock alternative while
keeping the public Pebble package version at 1.3.1.

## Analog clock

- Adds a Pro `Clock Face` selector with Digital and Analog options.
- Draws 12 / 3 / 6 / 9 cardinal numerals with a narrow industrial style.
- Draws 60 rectangularly projected tick marks with heavier hour marks.
- Hour and minute hands keep normal analog angles but project into the available
  rectangle, so they become longer toward the sides and shorter toward the top
  and bottom.
- Uses a compact center pivot with no oversized circle on the hour hand.
- Adds an optional `Analog Second Hand` setting. It subscribes to second ticks
  only while the analog second hand is actually enabled.
- Analog geometry is calculated from the live clock-layer bounds rather than
  fixed dimensions. This intentionally prepares the renderer for a later mode
  where the clock layer expands to the full 200x228 display when the data bars
  are hidden.

## Color behavior

- Cardinal numerals and tick marks use Clock Color.
- Hour and minute hands use Accent Color by default.
- When Separate Hour / Minute Colors is enabled, the analog hour and minute
  hands use those respective colors.
- The optional second hand uses a thin Clock Color shaft with an Accent Color
  tip.

KiezelPay production mode remains enabled. The existing WatchfaceSettings binary
schema/version is unchanged; the two analog preferences use separate persistent
keys for migration safety.
