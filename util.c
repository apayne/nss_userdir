/* nss_userdir/util.c
 * ------------------
 * 28 Apr 2002; mjp-userdir@pilcrow.madison.wi.us
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nss_userdir.h"

/* ud_read_file(filename, buffer, buffer size)
 *
 * Read the first line of .../filename, without \n, into the
 * given buffer.
 */
int
ud_readfile(const char *fn, char *buf, size_t sz) {
  FILE *f;
  size_t l;

  if ((f = fopen(fn, "r")) == NULL) return 0;
  if (fgets(buf, sz, f) == NULL) {
    fclose(f);
    return 0;
  }

  fclose(f);

  l = strlen(buf);
  if (l) {
    while (buf[l - 1] == '\n') { l--; }
    buf[l] = '\0';
  }
  return 1;
}

int
ud_readfile_uint(const char *fn, unsigned int *i) {
  char scratch[UINT_LEN];

  if (! ud_readfile(fn, scratch, UINT_LEN) ||
      ! strlen(scratch)) return 0;

  *i = (unsigned int) strtoul(scratch, NULL, 10);
  return 1;
}


