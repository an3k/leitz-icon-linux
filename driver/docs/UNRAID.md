# Unraid deployment — not released yet

**Current status:** The repository's development driver has not completed hardware validation. Do not treat a successful GitHub Actions run as a printer hardware qualification.

The existing, previously verified v0.2 CUPS image has been deleted from the user's Unraid installation. Deployment will use a **fresh build from this GitHub repository**, with no reliance on old appdata, stale locally tagged images or old containers.

## Intended Unraid container configuration

- Name: `cups`
- Network: `bridge`
- TCP port: host `631` → container `631`
- Persistence: `/mnt/cache/appdata/cups/etc` → `/etc/cups`
- Persistence: `/mnt/cache/appdata/cups/spool` → `/var/spool/cups`
- USB character device: host `/dev/usb/lpN` → container `/dev/usb/lpN` (verify that the host node actually belongs to the Leitz printer)
- Environment: `LEITZ_DEVICE=/dev/usb/lpN` (optional; defaults to `/dev/usb/lp0`; used only when the LeitzIcon queue is first created, and by `leitz-status`)
- Environment: `CUPS_ADMIN_USER`, `CUPS_ADMIN_PASSWORD` (at least 12 characters, unique and not committed)
- `--privileged` is not required by design.

**Security:** The setup exposes a CUPS administrative web UI. Restrict host port 631 to trusted networks. Do not publish it to the internet.

**Work in progress:** Once CI has passed and the driver is hardware-tested, this document will give a single version-pinned installation/update path. Until then, no specific image tag is claimed to be publicly available.

## Dynamische Druckprotokolle (Entwicklungsstand)

Das Docker-Image installiert beide `.so`-Module nach
`/usr/lib/leitz/protocols/`. Der gemeinsame CUPS-Treiber wählt per
`LeitzProtocol=Auto` zuerst Current und andernfalls Native; manuelle
Auswahl bleibt möglich. Die PPD muss für das Entfernen oder Ergänzen einer
Moduldatei nicht aktualisiert werden. Ein optionales Host-Bind-Mount auf
`/usr/lib/leitz/protocols` kann die Module austauschbar machen; es muss
dann die gewünschten `.so`-Dateien enthalten, weil es die im Image
enthaltenen Dateien verdeckt. Es ist kein zusätzlicher Dienst notwendig.
