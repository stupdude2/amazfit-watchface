# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-010
Based on: 1.3-dev-009

Backlight/settings stability:
- Returning from Settings/menu no longer calls request_light_with_fallback().
- window_appear() now synchronizes hidden/backlight-only UI to the actual system
  backlight state without generating a new light interaction.
- focus_handler() does the same.
- Only actual touch and Raise to Wake request the backlight.
- Prevents overlapping focus/window/touch/backlight transitions immediately
  after a settings save when top/bottom bars are backlight-only.
