# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-009
Based on: 1.3-dev-008

Crash/hitch hardening:
- Backlight/touch callbacks no longer call update_stepbar_layout() when only
  Top/Bottom bars are backlight-dependent.
- This prevents unnecessary full-clock redraws when hidden bars are revealed.
- Backlight-only stepbar modes still update correctly.
- update_stepbar_layout() now marks the clock dirty only if its frame actually
  changed.
- Time Style rendering itself is unchanged.
