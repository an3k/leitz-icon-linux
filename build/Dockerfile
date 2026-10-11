# The frozen upstream image's filesystem, CUPS startup command, and
# configuration are retained. Only production Leitz driver files are added.
ARG BASE_IMAGE=leitz-cups-frozen:source
FROM ${BASE_IMAGE} AS build
RUN apt-get update && apt-get install -y --no-install-recommends gcc make libcups2-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /work
COPY release/ /work/
RUN make clean all cups

FROM ${BASE_IMAGE}
COPY --from=build /work/leitz-cli /usr/lib/cups/filter/leitz-cli
COPY --from=build /work/leitz-rastertoicon /usr/lib/cups/filter/leitz-rastertoicon
COPY --from=build /work/cups/leitzusb /usr/lib/cups/backend/leitzusb
COPY --from=build /work/cups/leitz-status /usr/local/bin/leitz-status
COPY --from=build /work/cups/LeitzIcon.ppd /usr/share/ppd/LeitzIcon.ppd
RUN chmod 0755 /usr/lib/cups/filter/leitz-cli /usr/lib/cups/filter/leitz-rastertoicon /usr/local/bin/leitz-status \
    && chmod 0700 /usr/lib/cups/backend/leitzusb \
    && cupstestppd /usr/share/ppd/LeitzIcon.ppd
LABEL org.opencontainers.image.description="Unmodified frozen CUPS base with the approved Leitz Icon driver from main"
