# Big Time dev-19

- Added **Wrist Raise: Backlight Only** toggle.
- When enabled, the custom wrist-raise gesture lights the screen without revealing data configured for backlight-only display.
- A screen tap always turns on the backlight and reveals backlight-only data, whether the light was previously on or off.
- The light-only gate resets when the backlight turns off.
- Existing wrist-raise Off / Normal / Sensitive sensitivity control remains unchanged.

## dev-20
- Fixed the wrist-raise light-only flow when the display is already lit.
- A screen tap now latches an explicit data-reveal state for the remainder of the current backlight session.
- This prevents a delayed/repeated raise sample from re-hiding conditional bars/steps after the tap.
- The reveal latch resets when the backlight turns off.
