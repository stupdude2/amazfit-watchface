# Big Time 1.3 Development Build

Public/Rebble version: 1.3.0
Internal build: 1.3-dev-013
Based on: 1.3-dev-012

+2 forecast icon lifecycle fix:
- Current-weather refresh/re-tint no longer destroys the Tomorrow or +2 forecast GBitmaps.
- Each weather icon family now exclusively owns its own bitmap lifecycle.
- Background color changes explicitly rebuild/re-tint Current, Tomorrow, and +2 icons.
- Fixes Forecast +2 Hours icon popping in after returning from a menu.
- Bluetooth Colon and dev-011 repeat-save stability fixes are retained.
