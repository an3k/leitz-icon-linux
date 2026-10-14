#ifndef ORIGINAL_COMMAND_MAIN_FLOW_H
#define ORIGINAL_COMMAND_MAIN_FLOW_H
#include <stddef.h>
/* Functionally recovered control path of macOS 5.1.465 command2leitz _main,
 * x86_64 0x100000b10..0x1000011c0. The callback interface stands for CUPS
 * APIs and other OEM functions: NOT the native binary's _main ABI. */
typedef struct {
    void *context;
    void *(*file_open)(void *, const char *, const char *);
    void *(*file_stdin)(void *);
    void *(*ppd_open)(void *, const char *);
    int (*parse_options)(void *, const char *, void **);
    void (*mark_defaults)(void *, void *);
    void (*mark_options)(void *, void *, int, void *);
    const char *(*marked_choice)(void *, void *, const char *);
    void (*free_options)(void *, int, void *);
    void (*ppd_close)(void *, void *);
    const char *(*file_get_conf)(void *, void *, char *, size_t,
                                 char **, int *);
    int (*get_printer_status)(void *, int, double);
    int (*get_united_status)(void *, int, double);
    int (*get_firmware_version)(void *, int, double);
} classic_command_main_ops;

/* Mirrors original _main's defined branch outcomes. argc/argv passed by
 * caller exactly as from CUPS. No argument-count hardening or file-closing
 * side effects are added. stdout is managed inside the called status funcs. */
int classic_original_command_main(const classic_command_main_ops *ops,
                                  int argc, char **argv, const char *ppd_path);
#endif
