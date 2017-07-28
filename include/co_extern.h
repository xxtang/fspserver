#ifndef _FSP_CO_EXTERN_H_
#define _FSP_CO_EXTERN_H_ 1

/* common routines for both server and clients */

#ifndef HAVE_RANDOM
/* random.c */
void srandom (unsigned int);
char *initstate (unsigned int, char *, int);
char *setstate (char *);
long random (void);
#endif

/* udp_io.c */
int _x_udp (const char *, unsigned short *);
int _x_adr (const char *, int, struct sockaddr_in *);
int _x_select (fd_set *, long);

/* getsl.c */
char *getsl(char *s, int l);

#endif /* _FSP_CO_EXTERN_H_ */
