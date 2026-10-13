#define _POSIX_C_SOURCE 200809L
#include <cups/raster.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void test_raster(void);
struct cups_raster_s { unsigned page; unsigned pixel_reads; };
static struct cups_raster_s fake;
static int opened, closed;
static unsigned sizes[4];

cups_raster_t *cupsRasterOpen(int fd, cups_mode_t mode) {
    assert(fd==0 && mode==CUPS_RASTER_READ);
    ++opened;
    return &fake;
}
unsigned cupsRasterReadHeader2(cups_raster_t *r, cups_page_header2_t *h) {
    ++r->page;
    if (r->page == 3) return 0;
    h->cupsHeight = r->page==1 ? 3 : 1;
    h->cupsBytesPerLine = 4;
    h->NumCopies = r->page==1 ? 7 : 2;
    return 1;
}
unsigned cupsRasterReadPixels(cups_raster_t *r, unsigned char *b, unsigned n) {
    assert(b != NULL && n == 4);
    unsigned i = r->pixel_reads++;
    assert(i < 3);
    sizes[i] = n;
    if (i==1) return 0; /* immediate row break, next page still read */
    b[0] = 0xa5;
    return i==0 ? 1 : 4; /* short positive read is accepted */
}
void cupsRasterClose(cups_raster_t *r) {
    assert(r==&fake);
    ++closed;
}
int main(void) {
    FILE *capture=tmpfile();
    assert(capture);
    int saved=dup(fileno(stderr));
    assert(saved>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);
    test_raster();
    assert(fflush(stderr)==0);
    assert(dup2(saved,fileno(stderr))>=0);
    assert(close(saved)==0);
    rewind(capture);
    char log[256]={0};
    size_t n=fread(log,1,sizeof log-1,capture);
    log[n]=0;
    assert(fclose(capture)==0);
    assert(opened==1 && closed==1);
    assert(fake.page==3 && fake.pixel_reads==3);
    assert(sizes[0]==4 && sizes[1]==4 && sizes[2]==4);
    assert(strcmp(log,
        "\nraster_c_functions.c:358:test_raster(): PAGE: 1 7"
        "\nraster_c_functions.c:358:test_raster(): PAGE: 2 2")==0);
    puts("macOS original _test_raster behavior checks passed");
    return 0;
}
