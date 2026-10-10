# Leitz Icon: independent CUPS print server, no PDF cropper bundled.
FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends gcc make libcups2-dev ca-certificates \
 && rm -rf /var/lib/apt/lists/*
COPY src /build/src
COPY Makefile /build/Makefile
WORKDIR /build
RUN make all cups

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends cups cups-filters ghostscript ca-certificates bash passwd util-linux \
 && rm -rf /var/lib/apt/lists/*
COPY --from=build /build/leitz-cli /usr/lib/cups/filter/leitz-cli
COPY --from=build /build/leitz-rastertoicon /usr/lib/cups/filter/leitz-rastertoicon
COPY cups/leitzusb /usr/lib/cups/backend/leitzusb
COPY cups/leitz-status /usr/local/bin/leitz-status
COPY cups/LeitzIcon.ppd /usr/share/ppd/LeitzIcon.ppd
COPY cups/entrypoint.sh /usr/local/bin/leitz-cups-entrypoint
RUN chmod 0755 /usr/local/bin/leitz-cups-entrypoint /usr/local/bin/leitz-status \
    /usr/lib/cups/filter/leitz-cli /usr/lib/cups/filter/leitz-rastertoicon \
 && chmod 0700 /usr/lib/cups/backend/leitzusb \
 && mkdir -p /opt/leitz-cups-default \
 && cp -a /etc/cups/. /opt/leitz-cups-default/
EXPOSE 631
STOPSIGNAL SIGTERM
ENTRYPOINT ["/usr/local/bin/leitz-cups-entrypoint"]
