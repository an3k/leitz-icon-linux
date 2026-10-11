#define _POSIX_C_SOURCE 200809L
#include "../src/leitz_probe_io.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void pause_ms(long ms) {
    struct timespec t={.tv_sec=ms/1000,.tv_nsec=(ms%1000)*1000000L};
    while(nanosleep(&t,&t)<0 && errno==EINTR) {}
}

static void run(uint8_t cmd, const uint8_t *response, size_t length,
                int split, int expected, unsigned timeout_ms) {
    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM,0,fds)==0);
    const pid_t pid=fork();
    assert(pid>=0);
    if(pid==0) {
        close(fds[0]);
        uint8_t command[2];
        if(read(fds[1],command,sizeof command)!=2 || command[0]!=0x1b ||
           command[1]!=cmd) _exit(2);
        if(length) {
            if(split) {
                if(write(fds[1],response,1)!=1) _exit(3);
                pause_ms(20);
                if(write(fds[1],response+1,length-1)!=(ssize_t)(length-1)) _exit(4);
            } else if(write(fds[1],response,length)!=(ssize_t)length) _exit(5);
        }
        pause_ms(300);
        close(fds[1]);
        _exit(0);
    }
    close(fds[1]);
    int flags=fcntl(fds[0],F_GETFL,0);
    assert(flags>=0 && fcntl(fds[0],F_SETFL,flags|O_NONBLOCK)==0);
    uint8_t actual[LEITZ_PROBE_MAX_RESPONSE]={0};
    size_t n=0;
    int rc=leitz_probe_exchange(fds[0],cmd,actual,sizeof actual,&n,timeout_ms,100);
    assert(rc==expected);
    if(rc==0) { assert(n==length); assert(!memcmp(actual,response,length)); }
    close(fds[0]);
    int status=0;
    assert(waitpid(pid,&status,0)==pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status)==0);
}

static void test_partial_at_deadline(void) {
    int pair[2];
    assert(socketpair(AF_UNIX,SOCK_STREAM,0,pair)==0);
    pid_t child=fork();
    assert(child>=0);
    if(child==0) {
        close(pair[0]);
        uint8_t cmd[2];
        if(read(pair[1],cmd,2)!=2 || cmd[0]!=0x1b || cmd[1]!='A') _exit(2);
        pause_ms(100);
        const uint8_t part=0x03;
        if(write(pair[1],&part,1)!=1) _exit(3);
        pause_ms(300);
        close(pair[1]);
        _exit(0);
    }
    close(pair[1]);
    int old=fcntl(pair[0],F_GETFL,0);
    assert(old>=0 && fcntl(pair[0],F_SETFL,old|O_NONBLOCK)==0);
    uint8_t bytes[8]={0};size_t n=0;
    int rc=leitz_probe_exchange(pair[0],'A',bytes,sizeof bytes,&n,250,300);
    assert(rc==LEITZ_PROBE_TIMEOUT && n==1 && bytes[0]==0x03);
    close(pair[0]);
    int status;
    assert(waitpid(child,&status,0)==child);
    assert(WIFEXITED(status) && WEXITSTATUS(status)==0);
}

int main(void) {
    signal(SIGPIPE,SIG_IGN);
    const uint8_t stat[]={0x00}; /* Binary NUL is meaningful. */
    const uint8_t printed[]={0x00,0x00,0x01,0x2c};
    const uint8_t version[]={'5','.','1','.','4','6','5',0x00};
    uint8_t overflow[LEITZ_PROBE_MAX_RESPONSE+1];
    memset(overflow,0x41,sizeof overflow);
    run('A',stat,sizeof stat,0,0,500);
    run('P',printed,sizeof printed,1,0,500);
    run('V',version,sizeof version,1,0,500);
    run('A',NULL,0,0,LEITZ_PROBE_TIMEOUT,150);
    run('V',overflow,sizeof overflow,0,LEITZ_PROBE_OVERFLOW,500);
    test_partial_at_deadline();
    int pair[2];
    assert(socketpair(AF_UNIX,SOCK_STREAM,0,pair)==0);
    size_t count=123;
    assert(leitz_probe_exchange(pair[0],'X',overflow,10,&count,100,20)==-1);
    close(pair[0]);close(pair[1]);
    puts("PASS captured raw binary NUL, split replies, timeout, overflow, unknown query");
    return 0;
}
