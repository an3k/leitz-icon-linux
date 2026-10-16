#include "printer_data.h"
#include <string.h>
int leitz_win2018_retrieve_printer_value(
  const uint16_t *key,uint8_t *dst,size_t cap,
  size_t *len,uint32_t *type,
  leitz_win2018_printer_data_query query,void *ctx)
{
  if(!key || !len || !type || !query || (cap && !dst))return -1;
  uint8_t scratch[LEITZ_WIN2018_PRINTER_DATA_LIMIT]={0};
  uint32_t queried_type=0;
  size_t needed=0;
  /* Original GetPrinterDataW requests exactly 128 bytes and then
   * dangerously copies pcbNeeded bytes irrespective of status/capacity.
   * This portable reconstruction rejects that undefined behavior. */
  int status=query(ctx,key,&queried_type,scratch,sizeof scratch,&needed);
  if(status || needed>sizeof scratch || needed>cap || (needed && !dst))
    return -1;
  if(needed)memcpy(dst,scratch,needed);
  *len=needed;
  *type=queried_type;
  return 0;
}
