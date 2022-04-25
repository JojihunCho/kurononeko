#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char * argv[])
{
    int i = 1;
    while(i < 50)
    {
        i++;
        printf(1, "%d - ", uptime());
        printf(1, "%d\n", i);
    }
    exit();
}
