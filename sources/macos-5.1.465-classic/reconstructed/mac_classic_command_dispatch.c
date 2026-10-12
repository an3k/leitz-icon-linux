#include "mac_classic_command_dispatch.h"

static unsigned ascii_lower(unsigned c) {
  return c>='A' && c<='Z' ? c+('a'-'A') : c;
}
static int eq(const char *a, const char *b) {
  if (!a || !b) return 0;
  while (*a && *b) {
    if (ascii_lower((unsigned char)*a)!=ascii_lower((unsigned char)*b))return 0;
    ++a;++b;
  }
  return *a==0 && *b==0;
}
int mac_classic_command_select(const char *name, uint8_t *command) {
  if (!name || !command) return 0;
  if (eq(name,"ReportStatus")) { *command='A';return 1; }
  if (eq(name,"GetUnitedStatus")) { *command='U';return 1; }
  if (eq(name,"GetFirmwareVersion")) { *command='V';return 1; }
  return 0;
}
int mac_classic_should_send_esc_a(const char *choice) {
  return eq(choice,"YES");
}
