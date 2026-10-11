# Build without test sources; both protocol modules remain independent.
FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends gcc make libcups2-dev python3-minimal ca-certificates \
 && rm -rf /var/lib/apt/lists/*
COPY driver /build/driver
COPY Makefile /build/Makefile
WORKDIR /build
RUN make all cups probe ppd

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends cups cups-filters ghostscript ca-certificates bash passwd \
 && rm -rf /var/lib/apt/lists/*
COPY --from=build /build/leitz-cli /usr/lib/cups/filter/leitz-cli
COPY --from=build /build/leitz-rastertoicon /usr/lib/cups/filter/leitz-rastertoicon
COPY --from=build /build/driver/modules/ /usr/lib/leitz/protocols/
COPY driver/cups/leitzusb /usr/lib/cups/backend/leitzusb
COPY driver/cups/leitz-command /usr/lib/cups/filter/leitz-command
COPY driver/cups/leitz-status /usr/local/bin/leitz-status
COPY --from=build /build/leitz-probe /usr/local/bin/leitz-probe
COPY --from=build /build/driver/cups/LeitzIcon.ppd /usr/share/ppd/LeitzIcon.ppd
COPY driver/cups/entrypoint.sh /usr/local/bin/leitz-cups-entrypoint
RUN chmod 0755 /usr/local/bin/leitz-cups-entrypoint /usr/local/bin/leitz-status \
    /usr/lib/cups/filter/leitz-cli /usr/lib/cups/filter/leitz-rastertoicon \
    /usr/lib/cups/filter/leitz-command \
 && chmod 0700 /usr/local/bin/leitz-probe \
 && chmod 0700 /usr/lib/cups/backend/leitzusb \
 && mkdir -p /opt/leitz-cups-default \
 && cp -a /etc/cups/. /opt/leitz-cups-default/
EXPOSE 631
STOPSIGNAL SIGTERM
ENTRYPOINT ["/usr/local/bin/leitz-cups-entrypoint"]
