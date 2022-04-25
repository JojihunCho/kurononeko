#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char * argv[])
{
    int pid = fork();
    int i = 0;
    if (pid == 0) {
        while(i < 100) {
            printf(1,"Child\n");
            i++;            
        }
    } else if (pid > 0) {
        while(i < 100) {
            printf(1,"Parent\n");
            i++;
        }
    } else {
        printf(1,"fork failed\n");
    }
    return 0;
}

