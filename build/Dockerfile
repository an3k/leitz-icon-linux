# Source packages are built from the exact selected Debian bookworm revision.
FROM debian:bookworm-slim AS cups-builder
ARG CUPS_VERSION
RUN printf 'deb-src http://deb.debian.org/debian bookworm main\ndeb-src http://deb.debian.org/debian bookworm-updates main\ndeb-src http://deb.debian.org/debian-security bookworm-security main\n' > /etc/apt/sources.list.d/debian-src.list \
 && apt-get update \
 && apt-get install -y --no-install-recommends build-essential dpkg-dev ca-certificates \
 && apt-get build-dep -y cups \
 && rm -rf /var/lib/apt/lists/*
COPY sources/unpacked/ /sources/
WORKDIR /sources/cups-${CUPS_VERSION}
RUN dpkg-parsechangelog -S Version | grep -Fx "$CUPS_VERSION" \
 && dpkg-buildpackage --build=binary --no-sign --jobs=2 \
 && mkdir -p /packages \
 && mv /sources/*.deb /packages/

FROM debian:bookworm-slim AS driver-builder
RUN apt-get update && apt-get install -y --no-install-recommends gcc make libcups2-dev \
 && rm -rf /var/lib/apt/lists/*
COPY release/ /work/
WORKDIR /work
RUN make clean all cups

FROM debian:bookworm-slim
ARG CUPS_VERSION
COPY --from=cups-builder /packages/ /local-cups-packages/
RUN apt-get update \
 && apt-get install -y --no-install-recommends cups cups-filters ghostscript ca-certificates bash passwd \
 && apt-get install -y --no-install-recommends \
    /local-cups-packages/cups_*.deb \
    /local-cups-packages/cups-daemon_*.deb \
    /local-cups-packages/cups-client_*.deb \
    /local-cups-packages/cups-common_*.deb \
    /local-cups-packages/cups-core-drivers_*.deb \
    /local-cups-packages/cups-ipp-utils_*.deb \
    /local-cups-packages/cups-server-common_*.deb \
    /local-cups-packages/libcups2_*.deb \
    /local-cups-packages/libcupsimage2_*.deb \
 && test "$(dpkg-query -W -f='${Version}' cups)" = "$CUPS_VERSION" \
 && rm -rf /var/lib/apt/lists/* /local-cups-packages
COPY --from=driver-builder /work/leitz-cli /usr/lib/cups/filter/leitz-cli
COPY --from=driver-builder /work/leitz-rastertoicon /usr/lib/cups/filter/leitz-rastertoicon
COPY release/cups/leitzusb /usr/lib/cups/backend/leitzusb
COPY release/cups/leitz-status /usr/local/bin/leitz-status
COPY release/cups/LeitzIcon.ppd /usr/share/ppd/LeitzIcon.ppd
COPY build/entrypoint.sh /usr/local/bin/leitz-cups-entrypoint
RUN chmod 0755 /usr/lib/cups/filter/leitz-cli /usr/lib/cups/filter/leitz-rastertoicon \
    /usr/local/bin/leitz-status /usr/local/bin/leitz-cups-entrypoint \
 && chmod 0700 /usr/lib/cups/backend/leitzusb \
 && mkdir -p /opt/leitz-cups-default \
 && cp -a /etc/cups/. /opt/leitz-cups-default/ \
 && cupstestppd /usr/share/ppd/LeitzIcon.ppd
EXPOSE 631
STOPSIGNAL SIGTERM
ENTRYPOINT ["/usr/local/bin/leitz-cups-entrypoint"]
