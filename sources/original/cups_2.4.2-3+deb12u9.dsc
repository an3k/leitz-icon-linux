-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: cups
Binary: libcups2, cups, cups-bsd, cups-client, cups-common, cups-core-drivers, cups-daemon, cups-ipp-utils, cups-ppdc, cups-server-common, libcups2-dev, libcupsimage2, libcupsimage2-dev
Architecture: any all
Version: 2.4.2-3+deb12u9
Maintainer: Debian Printing Team <debian-printing@lists.debian.org>
Uploaders:  Till Kamppeter <till.kamppeter@gmail.com>, Thorsten Alteholz <debian@alteholz.de>,
Homepage: https://github.com/OpenPrinting/cups/
Standards-Version: 4.6.0
Vcs-Browser: https://salsa.debian.org/printing-team/cups
Vcs-Git: https://salsa.debian.org/printing-team/cups.git
Testsuite: autopkgtest
Build-Depends: autoconf, automake, debhelper-compat (= 13), dh-strip-nondeterminism, libavahi-client-dev, libavahi-common-dev, libdbus-1-dev, libgnutls28-dev, libkrb5-dev, libpam0g-dev, libpaper-dev, libsystemd-dev [linux-any], libtool, libusb-1.0-0-dev [!hurd-any], patch, pkg-config, po-debconf, po4a, zlib1g-dev
Build-Depends-Arch: dh-apparmor
Build-Conflicts: libgmp-dev (<< 2:6)
Package-List:
 cups deb net optional arch=any
 cups-bsd deb net optional arch=any
 cups-client deb net optional arch=any
 cups-common deb net optional arch=all
 cups-core-drivers deb net optional arch=any
 cups-daemon deb net optional arch=any
 cups-ipp-utils deb net optional arch=any
 cups-ppdc deb utils optional arch=any
 cups-server-common deb net optional arch=all
 libcups2 deb libs optional arch=any
 libcups2-dev deb libdevel optional arch=any
 libcupsimage2 deb libs optional arch=any
 libcupsimage2-dev deb libdevel optional arch=any
Checksums-Sha1:
 45055a7acb8da7cf743b6c572a601418a89f994f 8128785 cups_2.4.2.orig.tar.gz
 96713b90c9bb51b3f69ccc4e49913a7781d3ffe5 228 cups_2.4.2.orig.tar.gz.asc
 2784b9e680145e824bda13849a346869c7448cc1 391124 cups_2.4.2-3+deb12u9.debian.tar.xz
Checksums-Sha256:
 f03ccb40b087d1e30940a40e0141dcbba263f39974c20eb9f2521066c9c6c908 8128785 cups_2.4.2.orig.tar.gz
 1c116a5fb52e013f4569c3e4cb09a9934ea5e498b9e9d3b823bc2ae09066555b 228 cups_2.4.2.orig.tar.gz.asc
 7cdfaf0d9339a16723ee25e940e65172cd446baf35c476a2fa6de6f4730080c2 391124 cups_2.4.2-3+deb12u9.debian.tar.xz
Files:
 be5241e2c165eeb83b58aa32de26fb16 8128785 cups_2.4.2.orig.tar.gz
 eb60d5853f5988a4a91e3db1203fe508 228 cups_2.4.2.orig.tar.gz.asc
 908481a808d2244c34c0fc331b65e8d5 391124 cups_2.4.2-3+deb12u9.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQKnBAEBCgCRFiEEYgH7/9u94Hgi6ruWlvysDTh7WEcFAmjCv1JfFIAAAAAALgAo
aXNzdWVyLWZwckBub3RhdGlvbnMub3BlbnBncC5maWZ0aGhvcnNlbWFuLm5ldDYy
MDFGQkZGREJCREUwNzgyMkVBQkI5Njk2RkNBQzBEMzg3QjU4NDcTHGRlYmlhbkBh
bHRlaG9sei5kZQAKCRCW/KwNOHtYR4S1D/9E3nxWzruufv9SzyXvkLISkUoCoN6K
zW0kZyRFi6fij96HohG4nWUo46ttMG8cZ6zqtfTDgZrFNSzAdMy9H5jIzHTd+BIn
MQOxG8Y129CL8TBzonJ+teIBBdG7P6xQrFPmA0PwLWxdnNk2pH/FRpRTyEo8E5kI
nDiSLH6toH2C4S3XuePQhVV31bbEF2/X5kid+UFJBEik0DzRlv3UfR3mhR36jwUq
crFH2DXt69ecyK4dAjqhzGUbaxYA9yt/j9BLuiRf48w0FLVTHpvF0mSIaDywbFD+
gPFeUVAZ0upiNjrTv1d+m/t4ZgTsxY3YfIt6d8augS0IZ3zZaIp2+g8xsPA2jOt1
dBrC1XZezxmYcjfuEL8xxjMh2VZpz9He3nybf0MFMiQBd1gY9FQkW4bRgwb7Y0v7
TWjIGT99fHaoZpY8vqgCJe5u895/gyQYnUgZCk0eYzhaXKpUuoBD60LkEokZcuIO
XeSoCXUdgUOji5HrViTezdcQIWn04Y90+pDJGIaIWnUL63gLq5fznSoP+tEKcv2N
QyP6Ux/3+rS28Vd89oPD0kRvFJYlmJ/g+RNi+/GDgYEBmTl84FQ4ijGV/jkc/f8y
QvlO88ZTeADxn113psD4HwwyJIyBaOaa7UduNUgdAtkEkbZjzMm4qTrQt9XfC3it
QThYJNEFs+ay0Q==
=oU75
-----END PGP SIGNATURE-----
