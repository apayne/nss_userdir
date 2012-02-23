/* $Id: getpwent.c,v 1.2 2002/04/07 20:07:56 drench Exp $ */

/*

Copyright 2002 Daniel M. Rench.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED ``AS IS'' WITH NO EXPRESSED NOR IMPLIED
   WARRANTIES OF ANY KIND.

*/

/*
TO DO:

    * Check that the contents of /etc/users/{name}/NAME == {name}
    * Check permissions on key files (/etc/users/{name}/GID should
      not be owned by user {name}, etc.)
    * Should _cat allocate a bigger buffer?
    * Investigate pw_class; is returning NULL OK?
    * The many 'if's in a series could probably get reduced to a 'for' loop
    * use USERDIR more rather than the many hard-codes

*/

#include <stdio.h>
#include <pwd.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

#define USERDIR "/etc/users"

static struct passwd pw;
static struct stat statret;
static DIR *pwdir;

int
_chomp(char *l)
{
    int len;
    if ((len = strlen(l)) > 1) {
        if (l[len-1] == '\n') {
            l[len-1] = '\0';
            return(1);
        } else {
            return(0);
        }
    } else {
        l[0] = '\0';
        return(0);
    }
}

char *
_cat(char *fn)
{
    FILE *fp;
    char *buf;
    char *r;

    buf = (char *)malloc(128); /* 128? */
    if ((fp = fopen(fn, "r")) != NULL) {
        r = fgets(buf, 128, fp);
        fclose(fp);
        if (r != NULL) {
            _chomp(buf);
            return (char *) buf;
        }
    }
/*    free(buf);
    return (char *)NULL; */
    buf[0] = '\0';
    return (char *) buf;
}

int
_int_cat(char *fn)
{
    char *s;
    if ((s = _cat(fn)) == NULL) {
        return 0; /* be careful about setting UID/GID to 0 for !-e */
    } else {
        return atoi(s);
    }
}

int _exists(char *fn)
{
    return (stat(fn, &statret)) ? 0 : 1;
}

struct passwd *
getpwnam(const char *name)
{
    char *s;
    if (asprintf(&s, "%s/%s", USERDIR, name)) {
        stat(s, &statret);
        free(s);
        if (statret.st_mode & S_IFDIR) {
            if (asprintf(&s, "%s/%s/NAME", USERDIR, name)) {
                pw.pw_name = (char *)_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/PASSWD", USERDIR, name)) {
                pw.pw_passwd = (char *)_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/UID", USERDIR, name)) {
                pw.pw_uid = _int_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/GID", USERDIR, name)) {
                pw.pw_gid = _int_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/CHANGE", USERDIR, name)) {
                pw.pw_change = (time_t)_int_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/CLASS", USERDIR, name)) {
                pw.pw_class = (char *)_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/GECOS", USERDIR, name)) {
                pw.pw_gecos = (char *)_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/DIR", USERDIR, name)) {
                pw.pw_dir = (char *)_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/SHELL", USERDIR, name)) {
                pw.pw_shell = (char *)_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/EXPIRE", USERDIR, name)) {
                pw.pw_expire = (time_t)_int_cat(s); free(s);
            }
            if (asprintf(&s, "%s/%s/FIELDS", USERDIR, name)) {
                pw.pw_fields = _int_cat(s); free(s);
            }
            return(&pw);
        }
    }
    return (struct passwd *)NULL;
}

struct passwd *
getpwuid(uid_t uid)
{
    DIR *dir;
    struct dirent *de;
    char *s;

    if ((dir = opendir("/etc/users")) == NULL) {
        return (struct passwd *)NULL;
    }

    while ((de = readdir(dir)) != NULL) {
        if (de->d_name[0] != '.') {
            if (asprintf(&s, "%s/%s/UID", USERDIR, de->d_name)) {
                if ((_exists(s)) && (_int_cat(s) == uid)) {
                    closedir(dir);
                    return getpwnam(de->d_name);
                }
            }
        }
    }
    closedir(dir);
    return (struct passwd *)NULL;

}

struct passwd *
getpwent()
{
    struct dirent *de;

    if (pwdir == NULL) {
        if ((pwdir = opendir("/etc/users")) == NULL) {
            return (struct passwd *)NULL;
        }
    }

    de = readdir(pwdir);
    while ((de != NULL) && (de->d_name[0] == '.')) {
        de = readdir(pwdir);
    }

    if (de == NULL) {
        closedir(pwdir);
        pwdir = NULL;
        return (struct passwd *)NULL;
    } else {
        return (struct passwd *)getpwnam(de->d_name);
    }
}

int
setpassent(int stayopen) {
    if (pwdir != NULL)
        rewinddir(pwdir);
    return (stayopen == 0) ? closedir(pwdir) : 1;
}

void
endpwent()
{
    if (pwdir != NULL)
        closedir(pwdir);
}

void
setpwent()
{
    setpassent(0);
}
