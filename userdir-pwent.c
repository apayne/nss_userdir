/* nss_userdir
 * -----------
 * 30 Apr 2002; mjp-userdir@pilcrow.madison.wi.us
 *
 * Linux NSS implemenation of Dan Rench's userdir
 * (http://www.xnet.com/~drench/userdir)
 *
 * Far from perfect, far from pretty.  Some concerns:
 *
 *  threadsafety (mutex pwenum_dir?)
 *  username sanitization (getpwnam("../../path/to/badness"))
 *  permission bit checks? (../{U,G}ID mod restricted, e.g.)
 *  getspent/shadow support
 *  testing
 *
 */
#include <nss.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include "nss_userdir.h"

extern int errno;                /* NSS_STATUS_TRYAGAIN + ERANGE */
static DIR *pwenum_dir = NULL;   /* for getpwent() */

/* userdir_pwent("someuser", &passwd_struct, nss_buffer, nss_buffer_size)
 *
 * Fill in a passwd struct given a username, storing string members in
 * the supplied buffer.
 *
 * FIXME:  snprintf is too heavy for what we do -- just overwrite the end
 *         (final path component) of fn, which is guaranteed to be long
 *         enough.
 */

static enum nss_status
userdir_pwent(const char *u, struct passwd *pw, char *buf, size_t sz) {
  struct stat sb;
  size_t fnlen, l;
  enum nss_status ret = NSS_STATUS_NOTFOUND;
  char *fn = NULL;

  l = strlen(u);
  if (!l               ||
      !strcmp(u, ".")  ||
      !strcmp(u, "..") ||
      strstr(u, "../") != NULL) /* Someone is playing tricks on us */
    goto error;

  /* Allocate space for /etc/users/%USERNAME%/%USERDIR_FILE% */
  fnlen = UD_PARTIAL_PATHLEN + l;
  fn = (char *) malloc(fnlen);
  if (fn == NULL) goto error;
  memset((void *) fn, '\0', fnlen);

  /* Check for existence of userdir */
  snprintf(fn, fnlen, UD_USERDIR "/%s", u);
  if (stat(fn, &sb) == -1) {
    if (errno == ENOENT) goto out;
    goto error;
  }
  if (!S_ISDIR(sb.st_mode)) goto out;

  /* Fill in passwd struct */
  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "UID");
  if (!ud_readfile_uint(fn, &pw->pw_uid)) goto error;

  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "GID");
  if (!ud_readfile_uint(fn, &pw->pw_gid)) goto error;

/* convenience:
 *   shift buffer, decrement sz, and jump to moremem if needed
 */
#define pwent_shiftbuf()	\
  l = strlen(buf) + 1;		\
  buf += l;			\
  sz -= l;			\
  if (! sz) goto moremem;

  /* ??? Can we skip this?  ``u'' already contains this info */
  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "NAME");
  if (!ud_readfile(fn, buf, sz)) goto error;
  pw->pw_name = buf;
  pwent_shiftbuf();

  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "PASSWD");
  if (!ud_readfile(fn, buf, sz)) goto error;
  pw->pw_passwd = buf;
  pwent_shiftbuf();

  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "GECOS");
  if (!ud_readfile(fn, buf, sz)) goto error;
  pw->pw_gecos = buf;
  pwent_shiftbuf();

  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "DIR");
  if (!ud_readfile(fn, buf, sz)) goto error;
  pw->pw_dir = buf;
  pwent_shiftbuf();

  snprintf(fn, fnlen, UD_USERDIR "/%s/%s", u, "SHELL");
  if (!ud_readfile(fn, buf, sz)) goto error;
  pw->pw_shell = buf;
  pwent_shiftbuf();

  ret = NSS_STATUS_SUCCESS;  /* we made it! */

  out:
    if (fn != NULL) free(fn);
    return ret;

  error:
    ret = NSS_STATUS_UNAVAIL;
    goto out;

  moremem: /* glibc will keep trying buflen += 1024 */
    errno = ERANGE;
    ret = NSS_STATUS_TRYAGAIN;
    goto out;
}


enum nss_status _nss_userdir_getpwnam_r(
    const char *name,
    struct passwd *pw,
    char *buffer,
    size_t buflen) {

  return userdir_pwent(name, pw, buffer, buflen);

}

enum nss_status _nss_userdir_getpwuid_r(
    uid_t desired_uid,
    struct passwd *pw,
    char *buffer,
    size_t buflen) {

  DIR *dirp;
  struct dirent *d;
  uid_t test_uid;
  enum nss_status ret = NSS_STATUS_NOTFOUND;

  dirp = opendir(UD_USERDIR);
  if (dirp == NULL) return NSS_STATUS_UNAVAIL;

  while ((d = readdir(dirp)) != NULL) {
    if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
      continue;
    snprintf(buffer, buflen, UD_USERDIR "/%s/UID", d->d_name);
    if (!ud_readfile_uint(buffer, &test_uid)) {
      ret = NSS_STATUS_UNAVAIL;
      break;
    }
    if (desired_uid == test_uid) {
      ret = userdir_pwent(d->d_name, pw, buffer, buflen);
      break;
    }
  }

  closedir(dirp);

  return ret;

}

/* N.B. We ignore /etc/users/ files that are _not_ directories */

enum nss_status _nss_userdir_getpwent_r(
    struct passwd *pw,
    char *buffer,
    size_t buflen) {

  struct dirent *d;
  off_t offset;
  enum nss_status ret;

  if (pwenum_dir == NULL) {
    pwenum_dir = opendir(UD_USERDIR);
    if (pwenum_dir == NULL) return NSS_STATUS_UNAVAIL;
  }

  offset = telldir(pwenum_dir);

  while ((d = readdir(pwenum_dir)) != NULL) {
    if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
      continue;
    ret = userdir_pwent(d->d_name, pw, buffer, buflen);
    if (ret != NSS_STATUS_NOTFOUND) /* NOTFOUND == wasn't a directory */
      break;
    offset = telldir(pwenum_dir);
  }
  
  if (d == NULL) {
    closedir(pwenum_dir);
    pwenum_dir = NULL;
    return NSS_STATUS_NOTFOUND;
  }

  if (ret == NSS_STATUS_TRYAGAIN && errno == ERANGE) {
    /* NSS will summon us again, so be prepared! */
    seekdir(pwenum_dir, offset);
  } 

  return ret;
}

enum nss_status _nss_userdir_setpwent(void) {
  if (pwenum_dir != NULL)
    rewinddir(pwenum_dir);

  return NSS_STATUS_SUCCESS;
}

enum nss_status _nss_userdir_endpwent(void) {
  if (pwenum_dir != NULL) {
    closedir(pwenum_dir);
    pwenum_dir = NULL;
  }

  return NSS_STATUS_SUCCESS;
}

