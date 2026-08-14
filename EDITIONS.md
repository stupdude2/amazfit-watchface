# Big Time Free / Pro model

Big Time now ships as **one watchface** with one UUID. Do not create separate Standard
and Pro PBWs. Runtime entitlement controls the feature set.

- Free: 12/24-hour time format, Center 12 Hour Clock, 5000-step default goal.
- 48-hour Pro trial: explicitly started by the user from Settings.
- Pro: all customization features, permanently unlocked through KiezelPay.

`src/c/edition.h` intentionally keeps premium code compiled in (`WATCHFACE_PRO 1`) so
the same installed watchface can transition between Free, Trial, and licensed Pro.

The old `tools/set-edition.ps1` workflow is retired and must not be used to create a
separate Pro UUID.
