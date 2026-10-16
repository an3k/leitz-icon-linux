#include "filter_graphics_fragments.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t stream[1024];
    size_t total, count[16], calls, fail_on_call;
    int32_t fail_result;
} mock;

static int32_t emit(void *ctx, const uint8_t *data, size_t len)
{
    mock *m = ctx;
    assert(m->calls < 16 && m->total + len <= sizeof m->stream);
    m->count[m->calls++] = len;
    if (m->fail_on_call && m->calls == m->fail_on_call)
        return m->fail_result;
    if (len) memcpy(m->stream + m->total, data, len);
    m->total += len;
    return 0;
}
static void reset(mock *m) { memset(m, 0, sizeof *m); }
static void check(const mock *m, const uint8_t *data, size_t n)
{
    assert(m->total == n && !memcmp(m->stream, data, n));
}

int main(void)
{
    mock m;
    leitz_win2018_row_state state = {0};
    reset(&m);
    assert(leitz_win2018_emit_row_prefix(&state, 20, 2, emit, &m) == 0);
    const uint8_t a[] = {0x1b,'D',18, 0x1b,'B',2};
    check(&m,a,sizeof a);
    assert(m.calls == 2 && state.previous_trimmed_width == 18);

    reset(&m);
    assert(leitz_win2018_emit_row_prefix(&state, 23, 5, emit, &m) == 0);
    const uint8_t b[] = {0x1b,'B',5};
    check(&m,b,sizeof b);
    assert(m.calls == 1 && state.previous_trimmed_width == 18);

    reset(&m);
    assert(leitz_win2018_emit_row_prefix(&state, 260, 0, emit, &m) == 0);
    const uint8_t c[] = {0x1b,'D',4, 0x1b,'B',0};
    check(&m,c,sizeof c);
    assert(state.previous_trimmed_width == 260);

    reset(&m); m.fail_on_call = 1; m.fail_result = LEITZ_WIN2018_E_FAIL;
    assert(leitz_win2018_emit_row_prefix(&state, 40, 0, emit, &m)
           == LEITZ_WIN2018_E_FAIL);
    assert(m.calls == 1 && state.previous_trimmed_width == 260);

    reset(&m);
    assert(leitz_win2018_emit_row_prefix(&state, 3, 4, emit, &m)
           == LEITZ_WIN2018_E_FAIL && m.calls == 0);

    const uint8_t raw[] = {1,2,3,4};
    const uint8_t compressed[] = {0x99,0x88};
    reset(&m);
    assert(leitz_win2018_emit_row_payload(raw,4,compressed,2,emit,&m)==0);
    const uint8_t d[] = {0x17,0x99,0x88};
    check(&m,d,sizeof d);
    assert(m.calls==2 && m.count[0]==1 && m.count[1]==2);

    reset(&m);
    assert(leitz_win2018_emit_row_payload(raw,4,raw,4,emit,&m)==0);
    const uint8_t e[] = {0x16,1,2,3,4};
    check(&m,e,sizeof e);

    reset(&m); m.fail_on_call=1; m.fail_result=LEITZ_WIN2018_E_FAIL;
    assert(leitz_win2018_emit_row_payload(raw,4,compressed,2,emit,&m)
           == LEITZ_WIN2018_E_FAIL && m.calls==1);
    puts("Windows LeitzRp 2018 raster fragments: PASS");
    return 0;
}
