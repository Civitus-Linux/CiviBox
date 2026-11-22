#ifndef CIVIBOX_SHADOW_H
#define CIVIBOX_SHADOW_H

#include <stddef.h>

struct spwd {
    char *sp_namp;
    char *sp_pwdp;
    long sp_lstchg;
    long sp_min;
    long sp_max;
    long sp_warn;
    long sp_inact;
    long sp_expire;
    unsigned long sp_flag;
};

struct spwd *getspnam(const char *name);

#define AUTH_ERROR -1
#define AUTH_DENIED 0
#define AUTH_OK 1

int auth_check(const char *username, const char *password);

#endif
