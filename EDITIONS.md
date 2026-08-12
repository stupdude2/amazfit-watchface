# Standard / Pro edition workflow

This project uses one source tree for both editions.

From PowerShell in the project root:

```powershell
.\tools\set-edition.ps1 standard
```

or:

```powershell
.\tools\set-edition.ps1 pro
```

The script keeps three edition-sensitive values synchronized:

- `package.json`: display name and UUID
- `src/c/edition.h`: native C feature flag
- `src/pkjs/edition.js`: PebbleKit JS/config feature flag

Standard keeps the original app UUID. Pro uses a separate UUID so both editions can exist as distinct Pebble apps/watchfaces.

Current Pro-only customization:

- Accent Color

Before committing or building, run the edition command you intend to test/export, then confirm with `git diff` or `git status`.
