# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-011
Based on: 1.3-dev-010

Repeat-save fix:
- Snapshot settings before parsing each full Clay dictionary.
- Unchanged fields resent by Clay no longer cause the full layout/service
  pipeline to run again.
- Unchanged Time Zones, Hour/Minute colors, split colors, Seconds Colon,
  Time Style, and battery progress source no longer rewrite persistence.
- Color/background flags are recomputed from actual GColor changes.
- Diagnostic delta logging added for testing repeated saves.
