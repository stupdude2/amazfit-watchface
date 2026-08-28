# Big Time v1.3.1 — Rectangular Analog Dev 24

Reverted the experimental wrist-raise/backlight-only differentiation introduced after dev-18.

- Removed the "Wrist Raise: Backlight Only" setting and persisted key.
- Removed tap/accelerometer reveal handling and associated state.
- Wrist raise once again uses the normal interaction-light path.
- Any top bar, bottom bar, or step bar configured for "With Backlight" is revealed whenever the wrist gesture activates the backlight.
- Preserves all analog clock work through dev-18, including custom hand colors and automatic contrast for analog numerals/markers.

Public package version remains 1.3.1.
