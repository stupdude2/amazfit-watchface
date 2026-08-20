# KiezelPay integration handoff

This build contains the runtime Free/Pro enforcement layer, but deliberately
does NOT embed the merchant/account API key.

## Why
KiezelPay's Pebble licensing integration uses product-specific generated code
and performs critical entitlement/security checks in C. The merchant API key
is intended for account/sales API access and should not be shipped inside a
public PBW/JavaScript bundle.

## What is already implemented
- Runtime entitlement state: `s_pro_unlocked`
- Secure C-side gating of every premium customization message
- Free controls:
  - Time Format (12 Hour / 24 Hour)
  - Center 12 Hour Clock
- Free defaults:
  - 5000 step target
  - Center 12 Hour Clock enabled
- Watch->phone entitlement key: `PRO_LICENSE = 24`
- Phone->watch status request key: `LICENSE_CHECK = 25`
- KiezelPay callback hook:
  `watchface_kiezelpay_set_licensed(bool licensed)`

## Final integration
Create/configure the Big Time product in KiezelPay and download its
preconfigured Pebble/watchface integration package from the KiezelPay developer
dashboard. Add its library/source files to this project and route its licensed
callback to:

    watchface_kiezelpay_set_licensed(true);

and its unlicensed/expired callback to:

    watchface_kiezelpay_set_licensed(false);

Trial behavior can be mapped to either Free or Pro depending on the product's
trial strategy.

Do not commit the merchant API key to GitHub.
