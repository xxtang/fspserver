#ifndef _FSP_CLIENT_CONF_H_
#define _FSP_CLIENT_CONF_H_ 1

/****************************************************************************
 * Set this to point to the system wide default .fsp_prof file.             *
 * This file is used by fhostcmd to semi-automate the setting of            *
 * environment variable to ease the use of fsp                              *
 * It is only checked if neither ./.fsp_prof nor ~/.fsp_prof exist          *
 ****************************************************************************/
#define FSPRC SYSCONFDIR"/fsp_prof"
#define FSPSITESRC SYSCONFDIR"/fspsites" 

/****************************************************************************
 * The basename of the local startup file                                   *
 ****************************************************************************/
#define FSPPROF ".fsp_prof"
#define FSPSITES ".fspsites"

/****************************************************************************
 * Define the CLIENT_TIMEOUT if you want the client programs to time out
 * and abort after a certain period. Period is settable via an environment
 * variable FSP_TIMEOUT. See the fsp_env(7) for details
 ****************************************************************************/

/****************************************************************************
 * Define the following if you want fhostcmd to attempt to perform name     *
 * lookup on numeric host and numeric lookup on named hosts                 *
 ****************************************************************************/
#define HOST_LOOKUP 1

/* Key lock prefix, works best on local filesystem */
#ifndef FSP_KEY_PREFIX 
# define FSP_KEY_PREFIX "/tmp/.FSPL"
#endif

/* check if we have at least one lock type configured */
#ifdef FSP_USE_SHAREMEM_AND_SEMOP
#elif defined(FSP_NOLOCKING)
#elif defined(FSP_USE_LOCKF)
#elif defined(FSP_USE_FLOCK)
#elif defined(FSP_USE_SHAREMEM_AND_LOCKF)
#else
# error "Locking type is not configured"
#endif

#endif /* _FSP_CLIENT_CONF_H_ */
