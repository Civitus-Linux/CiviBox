#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "common.h"
#include "usage.h"
#include "auth.h"

typedef struct {
    int g;
    int G;
    int n;
    int r;
    int u;
    int h;
    char *user;
} Arg;

static void usage(void)
{
    xwrite("usage: id [-gGnru] [user]\n");
    xwrite("  -g    print group ID");
    xwrite("  -G    print all group IDs");
    xwrite("  -n    print names instead of numbers");
    xwrite("  -r    print real ID instead of effective ID");
    xwrite("  -u    print user ID");
    xwrite("  -h    Show this");
}


int id_cmd(int argc, char **argv)
{
    Arg arg = {0};

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            for (int j = 1; argv[i][j]; j++) {
                switch (argv[i][j]) {
                    case 'g': arg.g = 1; break;
                    case 'G': arg.G = 1; break;
                    case 'n': arg.n = 1; break;
                    case 'r': arg.r = 1; break;
                    case 'u': arg.u = 1; break;
                    case 'h': arg.h = 1; break;
                    default: usage(); return 2;
                }
            }
        } else {
            if (arg.user) return 2;
            arg.user = argv[i];
        }
    }

    if (arg.h) {
        usage();
        return 0;
    }

    if (arg.u + arg.g + arg.G > 1) return 2;

    uid_t uid = arg.r ? getuid() : geteuid();
    gid_t gid = arg.r ? getgid() : getegid();

    struct passwd *pw = arg.user ? getpwnam(arg.user) : getpwuid(uid);

    if (!pw) {
        fprintf(stderr, "id: unknown user\n");
        return 1;
    }

    if (arg.user) {
        uid = pw->pw_uid;
        gid = pw->pw_gid;
    }

    if (arg.u) {
        if (arg.n) xwrite(pw->pw_name);
        else xwrite("%u", (unsigned)uid);
        return 0;
    }

    if (arg.g) {
        if (arg.n) {
            struct group *gr = getgrgid(gid);
            if (!gr) return 1;
            xwrite(gr->gr_name);
        } else xwrite("%u", (unsigned)gid);
        return 0;
    }

    if (arg.G) {
        int n = getgroups(0, NULL);
        if (n < 0) return 1;

        gid_t groups[n + 1];
        groups[0] = gid;

        if (getgroups(n, groups + 1) < 0) return 1;

        for (int i = 0; i <= n; i++) {
            if (i) printf(" ");

            if (arg.n) {
                struct group *gr = getgrgid(groups[i]);
                if (gr) printf(gr->gr_name);
                else printf("%u", (unsigned)groups[i]);
            } else printf("%u", (unsigned)groups[i]);
        }

        printf("\n");
        return 0;
    }

    printf("uid=%u(%s) gid=%u", (unsigned)uid, pw->pw_name, (unsigned)gid);

    struct group *gr = getgrgid(gid);
    if (gr) printf("(%s)", gr->gr_name);

    int n = getgroups(0, NULL);
    if (n < 0) return 1;

    gid_t groups[n + 1];
    groups[0] = gid;

    if (getgroups(n, groups + 1) < 0) return 1;

    printf(" groups=");

    for (int i = 0; i <= n; i++) {
        if (i) printf(",");

        printf("%u", (unsigned)groups[i]);

        gr = getgrgid(groups[i]);
        if (gr) printf("(%s)", gr->gr_name);
    }

    printf("\n");
    return 0;
}
