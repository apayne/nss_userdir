/* nss_userdir; 30 Apr 2002 */
#ifndef _NSS_USERDIR_H
#define _NSS_USERDIR_H

#define UD_USERDIR "/etc/users"
#define UINT_LEN  6

/* Length of "/etc/users" + "/" + "/longest_filename" + NUL
 *
 * This + strlen(username) gives you the length of the userdir
 * pathname (incl. final NUL); longest base name is currently
 * "PASSWD".
 */

#define UD_PARTIAL_PATHLEN 19

int ud_readfile(const char *fn, char *buf, size_t sz);
int ud_readfile_uint(const char *fn, unsigned int *i);

#endif
