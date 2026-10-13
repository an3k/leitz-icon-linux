#include "classic_command_original_utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    for (unsigned bit=0;bit<64;bit++) {
        unsigned position=bit&31u;
        assert(classic_command_original_is_bit_set(1u<<position,bit)==1);
        assert(classic_command_original_is_bit_set(0u,bit)==0);
        assert(classic_command_original_is_bit_set(~(1u<<position),bit)==0);
    }
    FILE *dump=tmpfile();
    FILE *log=tmpfile();
    assert(dump!=NULL && log!=NULL);
    assert(fputs("dump",dump)>=0 && fputs("log",log)>=0);
    classic_command_original_flush(stdout,dump);
    classic_command_original_close_debug(&dump);
    classic_command_original_close_log(&log);
    assert(!dump && !log);
    classic_command_original_close_debug(&dump);
    classic_command_original_close_log(&log);
    puts("command2leitz original utility branch checks passed");
    return 0;
}
