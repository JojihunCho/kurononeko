#include "types.h"
#include "stat.h"
#include "user.h"

void
F_printf(int fd, const char *s, ...)
{
    write(fd, s, strlen(s));
}

int
main(int argc, char * argv[])
{
    int pid = fork();
    if (pid == 0) {
        while(1) {
            F_printf(1,"Child\n");
            yield();            
        }
    } else if (pid > 0) {
        while(1) {
            F_printf(1,"Parent\n");
            yield();
        }
    } else {
        F_printf(1,"fork failed\n");
    }
    return 0;
}

