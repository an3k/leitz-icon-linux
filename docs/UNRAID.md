# Leitz Icon on Unraid and existing CUPS servers

## Scope

The **driver** is an independent CUPS integration, not a CUPS server.
It can be installed in an existing Linux or Unraid CUPS environment.
This development branch has passed offline build and installation checks,
but its current modular printer output has not been qualified on hardware.

## Development driver: add to existing CUPS

The driver-only archive is produced by
`bash driver/install/package.sh`, or by the matching CI build.
It contains the compiled raster filter, USB backend, status command,
PPD, protocol libraries, and `install.sh`.

Install the extracted package **inside the existing CUPS environment**,
then add the Leitz Icon as an ordinary printer through CUPS administration.
The installer does not configure users, change `cupsd.conf`, create queues,
or alter other printers. A USB device must be available to the CUPS
environment as a real `/dev/usb/lpN` character device for actual printing;
the correct device must be identified on the host before exposing it.

## Frozen Unraid Community Applications CUPS base

The separate Git branch `cups` stores original Debian Bookworm CUPS
source packages in `sources/original/` and exactly one selected unpacked
revision in `sources/unpacked/cups-<version>/`. Its `upgrade` retrieves
new Debian versions; `restore` selects an archived revision; `build`
compiles CUPS locally and includes **only the approved `main` driver**.
It does not include development changes from `driver`.

The development driver can be installed with its independent driver-only
package into this Debian-CUPS image for controlled integration testing.
Such an installation must not be confused with the separately released
`main`-based CUPS image.

## Security and verification boundaries

Do not expose CUPS administration directly to the public internet.
The USB backend validates print data, printer identity, cartridge and
status before transmitting data. Completing a USB write does **not**
prove that the printer physically finished printing and cutting.
Until the batched hardware verification succeeds, neither the current
development driver nor the integrated image is production-qualified.
