#include "command2leitz_recovered.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

/* Mach-O __DATA,__data, 0x1000061d0 through 0x1000061e8.
 * These are the original string payloads, read at their pointed-to offsets. */
static const char printer_status_prefix[] = "PRINTER_STATUS_";
static const char united_status_prefix[] = "UNITED_STATUS_";
static const char firmware_version_prefix[] = "FIRMWARE_VERSION_";
static const char notification_domain[] = "com.xyron.Leitz-Icon-Software";

static void send_end_of_job(const classic_command_io *io,
                            int should_send_esc_a) {
  /* Vendor _EndOfJob ignores its only input argument. */
  (void)should_send_esc_a;
  io->put_character(io->context, 0x1b);
  io->put_character(io->context, 'Z');
  io->flush_output(io->context);
}

static void emit_state(const classic_command_io *io, unsigned char status) {
  io->message(io->context,
      "STATE: -com.leitz.out-of-paper-warning,media-empty-warning,media-jam-error,com.leitz.ready,com.leitz.general-error,com.leitz.paper-jam-error");
  if (status == 0x03) {
    io->message(io->context, "STATE: +com.leitz.ready");
  } else if (status == 0x20) {
    io->message(io->context,
                "STATE: +com.leitz.out-of-paper-warning,media-empty-warning");
  } else if (status == 0x30) {
    io->message(io->context,
                "STATE: +media-jam-error,com.leitz.paper-jam-error");
  } else if (status == 0x40) {
    io->message(io->context, "STATE: +com.leitz.general-error");
  }
}

int classic_command_get_status(const classic_command_io *io, char command,
                               const char *prefix, int should_send_esc_a,
                               double timeout) {
  /* OEM _GetPrinterStatus/_GetUnitedStatus/_GetFirmvareVersion
   * native disassembly and direct execution establish these literal
   * diagnostics BEFORE the ESC query and backchannel read. They have
   * no terminating newline, as in the manufacturer's executable. */
  switch (command) {
    case 'A':
      fprintf(stderr,
        "\ncommand2leitz.c:123:GetPrinterStatus(): send command GetPrinterStatus");
      break;
    case 'U':
      fprintf(stderr,
        "\ncommand2leitz.c:132:GetUnitedStatus(): send command GetUnitedStatus");
      break;
    case 'V':
      fprintf(stderr,
        "\ncommand2leitz.c:141:GetFirmvareVersion(): send command GetFirmvareVersion");
      break;
    default:
      break;
  }
  /* The original sends each byte with putchar, then fflush(stdout). */
  char reply[1024];
  int received;
  io->put_character(io->context, 0x1b);
  io->put_character(io->context, (unsigned char)command);
  io->flush_output(io->context);

  received = (int)io->read_backchannel(io->context, reply, 1024, timeout);
  if (received == -1 || received == 0) {
    if (received == 0)
      fprintf(stderr,"\ncommand2leitz.c:172:GetStatus(): empty back channel data");
    io->message(io->context, "ATTR: marker-message=NoData");
    return 0;
  }
  /* Faithfully reproduces the vendor's reply[received] store. When
   * received == 1024 the original overflows this buffer by one byte.
   * No additional bounds check is introduced in the recovered path. */
  reply[received] = '\0';
  if (command == 'A') {
    emit_state(io, (unsigned char)reply[0]);
  } else {
    char message[1025 + 32];
    /* Original cupsFilePrintf(leitz.log, "%s%s", prefix, reply) and
     * fprintf(stderr, "ATTR: marker-message=\"%s%s\"\n", ...).
     * Unlike the experimental implementation there is no hex escaping. */
    if (io->write_leitz_log) {
      char contents[1025 + 32];
      snprintf(contents, sizeof(contents), "%s%s", prefix, reply);
      if (io->write_leitz_log(io->context, contents)) {
        if (io->post_notification)
          io->post_notification(io->context, notification_domain);
        snprintf(message, sizeof(message), "ATTR: marker-message=\"%s%s\"",
                 prefix, reply);
        io->message(io->context, message);
      }
    }
  }
  send_end_of_job(io, should_send_esc_a);
  return 1;
}

int classic_command_dispatch(const classic_command_io *io, const char *name,
                             const char *value, int should_send_esc_a) {
  /* Original _main uses strcasecmp for all three names. The first branch
   * jumps straight to its common epilogue with exit code 1, irrespective
   * of _GetStatus's return value; the other two return to the read loop. */
  if (strcasecmp(name, "ReportStatus") == 0) {
    classic_command_get_status(io, 'A', printer_status_prefix,
                               should_send_esc_a, 7.0);
    return 1;
  }
  if (strcasecmp(name, "GetUnitedStatus") == 0) {
    classic_command_get_status(io, 'U', united_status_prefix,
                               should_send_esc_a, 7.0);
  } else if (strcasecmp(name, "GetFirmwareVersion") == 0) {
    classic_command_get_status(io, 'V', firmware_version_prefix,
                               should_send_esc_a, 7.0);
  }
  (void)value;
  return 0;
}
