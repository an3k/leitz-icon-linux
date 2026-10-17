#include "gpd_job_sequence.h"
#include "gpd_commands.h"
#include "gpd_media_catalog.h"

static int send_gpd_command(leitz_win2018_gpd_command command,
                            leitz_win2018_gpd_job_write write,void *context)
{
    uint8_t bytes[122];
    size_t length=0;
    if (!write || leitz_win2018_gpd_copy_command(command,bytes,sizeof bytes,
                                                 &length)!=0)
        return -1;
    return write(context,bytes,length)==0 ? 0 : -1;
}

int leitz_win2018_gpd_begin_document(leitz_win2018_gpd_job_write write,
                                     void *context)
{
    return send_gpd_command(LEITZ_WIN2018_GPD_START_DOCUMENT,write,context);
}

int leitz_win2018_gpd_setup_page(const char *original_paper_id,
                                 leitz_win2018_gpd_resolution resolution,
                                 leitz_win2018_gpd_job_write write,
                                 void *context)
{
    const leitz_win2018_gpd_media_entry *paper =
        leitz_win2018_gpd_media_by_name(original_paper_id);
    if (!write || !paper || (resolution!=LEITZ_WIN2018_RES_OPTION1 &&
                              resolution!=LEITZ_WIN2018_RES_OPTION2))
        return -1; /* Validate the *whole* request before writing. */

    if (send_gpd_command(LEITZ_WIN2018_GPD_START_PAGE,write,context)!=0)
        return -1;
    if (write(context,paper->command,paper->command_length)!=0)
        return -1;
    /* Manufacturer resolution CmdSelect; other GPD halftone options
     * have no literal CmdSelect and are handled by Windows host. */
    const uint8_t resolution_command[2]={0x1b,
        resolution==LEITZ_WIN2018_RES_OPTION1 ? 'h' : 'i'};
    return write(context,resolution_command,sizeof resolution_command)==0 ? 0:-1;
}

int leitz_win2018_gpd_end_page(void)
{
    return 0; /* Original CmdEndPage *Cmd is the empty string. */
}

int leitz_win2018_gpd_end_job(leitz_win2018_gpd_job_write write,
                              void *context)
{
    return send_gpd_command(LEITZ_WIN2018_GPD_END_JOB,write,context);
}
