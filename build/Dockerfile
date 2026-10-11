# Debian Bookworm CUPS compiled from the archived, patched Debian source.
# --target cups-base: no Leitz driver, PPD, backend, or printer queue.
# --target cups-with-driver: install the selected main/dev driver-only package.
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

FROM debian:bookworm-slim AS cups-base
ARG CUPS_VERSION
COPY --from=cups-builder /packages/ /local-cups-packages/
RUN apt-get update \
 && apt-get install -y --no-install-recommends cups cups-filters ghostscript ca-certificates bash passwd \
 && apt-get install -y --allow-downgrades --no-install-recommends \
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
COPY build/entrypoint.sh /usr/local/bin/cups-entrypoint
RUN chmod 0755 /usr/local/bin/cups-entrypoint \
 && mkdir -p /opt/cups-default \
 && cp -a /etc/cups/. /opt/cups-default/
ENV LEITZ_CUPS_DRIVER=none
EXPOSE 631
STOPSIGNAL SIGTERM
ENTRYPOINT ["/usr/local/bin/cups-entrypoint"]

FROM debian:bookworm-slim AS driver-builder
RUN apt-get update && apt-get install -y --no-install-recommends gcc make python3 libcups2-dev \
 && rm -rf /var/lib/apt/lists/*
COPY release/ /work/
WORKDIR /work
RUN bash install/package.sh \
 && cp dist/leitz-icon-driver-*.tar.gz /tmp/leitz-driver.tar.gz

FROM cups-base AS cups-with-driver
ARG DRIVER_MODE=main
COPY --from=driver-builder /tmp/leitz-driver.tar.gz /tmp/leitz-driver.tar.gz
RUN mkdir -p /tmp/leitz-install \
 && tar -xzf /tmp/leitz-driver.tar.gz -C /tmp/leitz-install \
 && bash /tmp/leitz-install/install.sh \
 && test -x /usr/lib/cups/filter/leitz-rastertoicon \
 && test -x /usr/lib/cups/filter/leitz-cli \
 && test -x /usr/lib/cups/backend/leitzusb \
 && test -s /usr/share/ppd/LeitzIcon.ppd \
 && ppd_result=$(cupstestppd /usr/share/ppd/LeitzIcon.ppd 2>&1 || true) \
 && printf '%s\n' "$ppd_result" \
 && printf '%s\n' "$ppd_result" | grep -Fq '/usr/share/ppd/LeitzIcon.ppd: PASS' \
 && rm -rf /tmp/leitz-driver.tar.gz /tmp/leitz-install
ENV LEITZ_CUPS_DRIVER=${DRIVER_MODE}
