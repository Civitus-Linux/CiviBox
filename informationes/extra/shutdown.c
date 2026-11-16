#include <unistd.h>
#include <sys/syscall.h>
#include <sys/reboot.h>
#include <stdio.h>

int shutdown_cmd()
{
    if (geteuid() == 0)
    {
        puts("Are you root?");
        return 1;
    }
    sync();
    reboot(RB_POWER_OFF);
    return 0;
}
