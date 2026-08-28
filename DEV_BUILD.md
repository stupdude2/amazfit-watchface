# Big Time Development Build

Public package version remains **1.3.1** while analog development continues.

## Rectangular Analog dev-8

- Fixes the step/progress bar when its mode is **With Backlight** while using the analog clock.
- Backlight events now update the step-bar layer at the same moment the analog clock begins its resize animation.
- The analog face still expands into space released by hidden top/bottom bars and contracts when conditional bars return.
- Existing analog dial geometry, hand scaling, hand layering, and second-hand option are unchanged.

## Rectangular analog dev-9
- In analog mode, the visible steps/progress bar now follows the effective top/bottom bar layout.
- When the bottom data bar is hidden and steps remain visible, the steps bar is pinned to the bottom edge and the expanded analog face ends directly above it.
- When the top data bar is hidden with an above-clock steps mode, the same rule applies at the top edge.


## Analog dev-10
- Fixed CloudPebble compile failure introduced in dev-9 by adding forward declarations for `header_is_effectively_visible()` and `footer_is_effectively_visible()` before `update_analog_stepbar_layer()` uses them.
- No behavioral/layout changes from dev-9.
