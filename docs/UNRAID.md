# Leitz Icon v0.4.1 — installation in an existing CUPS environment

## Scope

This guide applies only to the older `main` driver. It does not
install a CUPS server or a replacement Docker container. Existing
printers and their configurations are not changed by the installer.

## Installation

1. Build or obtain the archive
   `leitz-icon-driver-main-linux-amd64.tar.gz` from the `main`
   CI workflow (see [BUILD.md](BUILD.md)).
2. Extract it in a temporary directory **inside the existing CUPS
   environment**.
3. Run the included `install.sh` as root in that environment.
4. Add the Leitz Icon normally through the existing CUPS administration
   interface, selecting the installed `LeitzIcon.ppd`.

The installer adds a CUPS raster filter, the `leitzusb` backend, its
validator, a cartridge status command and the PPD. It does not create
or modify print queues, accounts, server configuration or passwords.

## USB access and printer setup

This legacy backend works with Linux printer-class devices at
`/dev/usb/lpN`. It validates USB VID:PID `289e:1000` before sending
anything. In a container, the selected character device must be made
available to the existing CUPS environment by its administrator.

For the installed PPD, the backend URI syntax is
`leitzusb:/dev/usb/lpN?sku=auto` (or an explicitly selected eight-digit
SKU). Replace `N` with the device's actual number. The backend
detects cartridges using `ESC U` before transmission. Some media
width mappings are derived from vendor sources, not physical tests.

For a new queue, use CUPS' `printer-error-policy=stop-printer`
setting to prevent unattended repeated attempts after uncertain
transmission errors. This option is set when the queue is configured,
not by `install.sh`.

## Limitations

This driver implements the legacy v0.4.1 protocol and its 76-profile
media catalog, including continuous-stock handling and custom page
selection. The separate Native Linux encoder and the modular protocol
loader exist only in the development branch, not here. The historical
170 mm calibration print is not a general length restriction.

A successful USB transfer is **not** proof of mechanical print
completion or automatic cutting. Neither this legacy version nor
all supported media have been hardware-qualified for production.
