#ifndef _FSP_SERVER_CONF_H_
#define _FSP_SERVER_CONF_H_

/****************************************************************************
 * Set this to the location of the fspd.conf file on your system            *
 - by xxfan 2016-6-14
#define CONF_FILE  "/var/config/fspd.conf"
+
 ****************************************************************************/

#define CONF_FILE  "/var/config/fspd.ini"

/****************************************************************************
 * Set this value to the maximum number of open files you wish your system  *
 * to keep around at any given time.  The smaller this number is, the more  *
 * likely the server is to be opening and closing files, but the less file  *
 * descriptors need to be taken up by the server itself                     *
 * Five seems to work reasonably well on my system                          *
 ****************************************************************************/
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

/****************************************************************************
 * DEFAULT_DIRLISTCACHE_SIZE should be set to contain the number of
 * listings you want to held in memory cache.
 * Dirlisting building needs a lot of syscalls, so it should be set to
 * some higher number.
 * Following setting is a minimum recommended size.
 ****************************************************************************/

#define DEFAULT_DIRLISTCACHE_SIZE 100

/****************************************************************************
 * DEFAULT_DIRSTATCACHE_SIZE should be set to contain the number of dirs    *
 * you want to held in dirstat memory cache. This cache avoids calling stat *
 * on directory and loading access perms. This operation is far less
 * expensive than listing a directory, so if can be a lower number.
 ***************************************************************************/

#define DEFAULT_DIRSTATCACHE_SIZE 30

/****************************************************************************
 * Set this value to the maximum number of open files you wish your system  *
 * to keep around at any given time.  The smaller this number is, the more  *
 * likely the server is to be opening and closing files, but the less file  *
 * descriptors need to be taken up by the server itself.                    *
 ****************************************************************************/
#define DEFAULT_FPCACHE_SIZE 20

/* THCCOUNT is the number of seconds used to compute average throughput.
 * 10 seconds seems to be a good value
 */
#define THCCOUNT 10

#endif /* _FSP_SERVER_CONF_H_ */
