# Big Time v3.3.3 — Production Release

Publishable production build.

Included changes:
- Fixes Pro settings visibility when a KiezelPay trial is active.
- Rebuilds the Clay settings definition from the current entitlement whenever Settings opens, preventing stale Free-only controls during an active trial or paid Pro state.
- Tap/Shake visibility support for Top Bar and Bottom Bar.
- Combined Tap/Shake/Backlight visibility option.
- Step Progress Bar style and visibility are separate settings, with the same visibility choices as Top/Bottom bars.
- Wrist gesture can reveal interaction bars independently of whether ambient-light logic actually illuminates the backlight.
- Optional expandable digital clock reclaims vertical space when bars/steps are hidden.
- Expanded digital clock animates with the same 260 ms ease-in/out behavior used by the analog face.
- Expanded digital clock uses the same 6 px outer top/bottom edge margin as the analog face.
- Saved settings force immediate structural/layout reconciliation, including Step Bar visibility and dynamic clock geometry.

Publishing safeguards:
- CONFIG_TEST_MODE = false
- KiezelPay production integration enabled
- KIEZELPAY_LOGGING = false
- UUID unchanged: ced51275-d445-4b7e-89f6-9e41110ed4da
- Package version: 3.3.3
