#include "paper_geometry.h"
int leitz_win2018_collect_paper_geometry(
  const uint16_t *ids, size_t count,
  leitz_win2018_paper_geometry *out, size_t capacity,
  leitz_win2018_select_paper_fn select,
  leitz_win2018_get_cap_fn get, void *ctx)
{
  const int indices[] = {112,113,110,111,8,10};
  if(count>capacity || (count && (!ids || !out || !select || !get)))
    return -1;
  for(size_t i=0;i<count;++i) {
    if(select(ctx,ids[i])!=0) return -1;
    int32_t values[6];
    for(size_t j=0;j<6;++j)
      if(get(ctx,indices[j],&values[j])!=0) return -1;
    const leitz_win2018_paper_geometry record = {
      ids[i],values[0],values[1],values[2],values[3],values[4],values[5]
    };
    out[i]=record;
  }
  return 0;
}
