#include "oem_slot8_finish.h"

int32_t leitz_rp_oem_slot8_finish(leitz_rp_oem_write_state *writer,
                                  const leitz_rp_oem_slot8_arguments *args)
{
    if (!writer || !args || !args->free_resource || !args->allocated_block ||
        !writer->write || !writer->report ||
        (writer->architecture != LEITZ_RP_WRITE_X86 &&
         writer->architecture != LEITZ_RP_WRITE_X64))
        return (int32_t)LEITZ_RP_WRITE_E_FAIL;

    const uint32_t target=(uint32_t)args->medium_word_at_0c * 5u + 16u;
    const uint64_t accumulated=writer->architecture==LEITZ_RP_WRITE_X86 ?
        (uint32_t)writer->accumulated_bytes : writer->accumulated_bytes;
    if (accumulated == target) {
        const uint8_t esc=0x1b;
        (void)leitz_rp_oem_write_data(writer, &esc, 1u);
    }
    args->free_resource(args->free_context,args->nested_allocated_resource);
    args->free_resource(args->free_context,args->allocated_block);
    return (int32_t)LEITZ_RP_WRITE_S_OK;
}
