    /*********************************************************************\
    *  Copyright (c) 2004-2005 by Radim Kolar                                  *
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include <stdlib.h>
#include <time.h>
#include "client_def.h"
#include "c_extern.h"
#include "my-string.h"

#ifndef FSP_NOLOCKING
static char key_string[sizeof(FSP_KEY_PREFIX)+32];

static char code_str[] =
    "0123456789:_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void make_key_string(	unsigned long server_addr,
			   	unsigned long server_port)
{
  unsigned long v1, v2;
  char *p;

  strcpy(key_string,FSP_KEY_PREFIX);
  for(p = key_string; *p; p++)
      ;
  v1 = server_addr;
  v2 = server_port;

  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  v1 = v1 | (v2 << (32-3*6));
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p++ = code_str[v1 & 0x3f]; v1 >>= 6;
  *p   = 0;
}
#endif

/********************************************************************/
/******* For those systems that has flock function call *************/
/********************************************************************/
#ifdef FSP_USE_FLOCK

#include <sys/file.h>

int key_persists = 1;
static unsigned int lock_fd;
static unsigned int okey;

unsigned short client_get_key (void)
{
  if(flock(lock_fd,LOCK_EX) == -1) {
    perror("flock");
    exit(EX_OSERR);
  }
  if(read(lock_fd,&okey,sizeof(okey)) == -1) {
    perror("read"); exit(EX_OSERR);
  }
  if(lseek(lock_fd,0L,0) == -1) {
    perror("seek");
    exit(EX_OSERR);
  }
  return(okey);
}

void client_set_key (unsigned short nkey)
{
  unsigned int key;
  key=nkey;  
  if(write(lock_fd,&key,sizeof(key)) == -1) {
    perror("write");
    exit(EX_OSERR);
  }
  if(lseek(lock_fd,0L,0) == -1) {
    perror("seek");
    exit(EX_OSERR);
  }
  if(flock(lock_fd,LOCK_UN) == -1) {
    perror("unflock");
    exit(EX_OSERR);
  }
}

void client_init_key (unsigned long server_addr,
			    unsigned long server_port,
			    unsigned short key)
{
  mode_t omask;
  okey = key;

  make_key_string(server_addr,server_port);

  omask = umask(0);
  lock_fd = open(key_string,O_RDWR|O_CREAT,0666);
  umask(omask);
}

void client_destroy_key(void)
{
    (void)close(lock_fd);
    unlink(key_string);
}
#endif
/********************************************************************/
/******* For those systems that has lockf function call *************/
/********************************************************************/
#ifdef FSP_USE_LOCKF

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int key_persists = 1;
static unsigned int lock_fd;
static unsigned int okey;

unsigned short client_get_key (void)
{
  if(lockf(lock_fd,F_LOCK,sizeof(okey)) == -1) {
    perror("lockf");
    exit(EX_OSERR);
  }
  if(read(lock_fd,&okey,sizeof(okey)) == -1) {
    perror("read");
    exit(EX_OSERR);
  }
  if(lseek(lock_fd,0L,0) == -1) {
    perror("seek");
    exit(EX_OSERR);
  }
  return(okey);
}

void client_set_key (unsigned short nkey)
{
  unsigned int key;
  key=nkey;  
  if(write(lock_fd,&key,sizeof(key)) == -1) {
    perror("write");
    exit(EX_OSERR);
  }
  if(lseek(lock_fd,0L,0) == -1) {
    perror("seek");
    exit(EX_OSERR);
  }
  if(lockf(lock_fd,F_ULOCK,sizeof(key)) == -1) {
    perror("unlockf");
    exit(EX_OSERR);
  }
}

void client_init_key (unsigned long server_addr,
			    unsigned long server_port,
			    unsigned short key)
{
  mode_t omask;
  okey = key;

  make_key_string(server_addr,server_port);

  omask = umask(0);
  lock_fd = open(key_string,O_RDWR|O_CREAT,0666);
  umask(omask);
}

void client_destroy_key(void)
{
    (void)close(lock_fd);
}
#endif
/********************************************************************/
/******* For those systems that has SysV shared memory + lockf ******/
/********************************************************************/
#ifdef FSP_USE_SHAREMEM_AND_LOCKF

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/ipc.h>
#include <sys/shm.h>

int key_persists = 1;
static unsigned int *share_key;
static unsigned int lock_fd;
static int   lock_shm;

unsigned short client_get_key (void)
{
  if(lockf(lock_fd,F_LOCK,2) == -1) {
    perror("lockf");
    exit(EX_OSERR);
  }
  return(*share_key);
}

void client_set_key (unsigned short key)
{
  *share_key = key;
  if(lockf(lock_fd,F_ULOCK,2) == -1) {
    perror("unlockf");
    exit(EX_OSERR);
  }
}

void client_init_key (unsigned long server_addr,
			    unsigned long server_port,
			    unsigned short key)
{
  mode_t omask;
  key_t lock_key;

  make_key_string(server_addr,server_port);

  omask = umask(0);
  lock_fd = open(key_string,O_RDWR|O_CREAT,0666);
  umask(omask);

  if((lock_key = ftok(key_string,238)) == -1) {
    perror("ftok");
    exit(EX_OSERR);
  }
  if((lock_shm = shmget(lock_key,2*sizeof(unsigned int),IPC_CREAT|0666)) == -1) {
    perror("shmget");
    exit(EX_OSERR);
  }
  if(!(share_key = (unsigned int *) shmat(lock_shm,(char*)0,0))) {
    perror("shmat");
    exit(EX_OSERR);
  }
}

void client_destroy_key(void)
{
    (void)close(lock_fd);
    if (shmdt((char *)share_key) < 0)
    {
	perror("shmdt");
    }
    shmctl(lock_shm,IPC_RMID,NULL); 
}
#endif
/********************************************************************/
/******* For those who does not want to use locking *****************/
/********************************************************************/
#ifdef FSP_NOLOCKING

int key_persists = 0;
static unsigned short okey;

unsigned short client_get_key (void)
{
  return(okey);
}

void client_set_key (unsigned short key)
{
  okey = key;
}

void client_init_key (unsigned long server_addr,
			    unsigned long server_port,
			    unsigned short key)
{
  okey = key;
}

void client_destroy_key(void)
{
    return;
}
#endif
/********************************************************************/
/******* For those systems that has SysV shared memory + semop ******/
/********************************************************************/
#ifdef FSP_USE_SHAREMEM_AND_SEMOP

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#ifdef _SEM_SEMUN_UNDEFINED
union semun
{
  int val;
  struct semid_ds *buf;
  unsigned short int *array;
  struct seminfo *__buf;
};
#endif

int key_persists = 1;
static unsigned int *share_key;
static int   lock_shm;
static int   lock_sem;

unsigned short client_get_key (void)
{
  struct sembuf sem;
  sem.sem_num = 0;
  sem.sem_op = -1;
  sem.sem_flg = SEM_UNDO;
  
  if(semop(lock_sem,&sem,1) == -1 )
  {
      perror("semop");
      exit(EX_OSERR);
  }
  return(*share_key);
}

void client_set_key (unsigned short key)
{
  struct sembuf sem;

  sem.sem_num = 0;
  sem.sem_op = 1;
  sem.sem_flg = SEM_UNDO;

  *share_key = key;
  if(semop(lock_sem,&sem,1) == -1) {
    perror("semop");
    exit(EX_OSERR);
  }
}

void client_init_key (unsigned long server_addr,
			    unsigned long server_port,
			    unsigned short key)
{
  mode_t omask;
  key_t lock_key;
  int fd;
  union semun sun;
  struct sembuf sem;

  make_key_string(server_addr,server_port);

  omask = umask(0);
  fd = open(key_string,O_RDWR|O_CREAT,0666);
  umask(omask);
  close(fd);

  if((lock_key = ftok(key_string,238)) == -1) {
    perror("ftok");
    exit(EX_OSERR);
  }
  if((lock_shm = shmget(lock_key,2*sizeof(unsigned int),IPC_CREAT|0666)) == -1) {
    perror("shmget");
    exit(EX_OSERR);
  }
  if(!(share_key = (unsigned int *) shmat(lock_shm,(char*)0,0))) {
    perror("shmat");
    exit(EX_OSERR);
  }

  if((lock_sem = semget(lock_key,0,0)) == -1) {
      /* create a new semaphore and init it */
      if((lock_sem = semget(lock_key,2,IPC_CREAT|0666)) == -1) {
	  perror("semget");
      }
      /* we need to init this semaphore */
      sun.val=1;
      if(semctl(lock_sem,0,SETVAL,sun) == -1)
      {
	  perror("semctl setval");
	  exit(EX_OSERR);
      }
      *share_key = key;
  }

  /* increase in use counter */
  sem.sem_num = 1;
  sem.sem_op = 1;
  sem.sem_flg = SEM_UNDO;

  if(semop(lock_sem,&sem,1) == -1) {
      perror("semop");
      exit(EX_OSERR);
  }
}

void client_destroy_key(void)
{
    int rc;
    
    if (shmdt((char *)share_key) < 0)
    {
	perror("shmdt");
	exit(EX_OSERR);
    }
    /* check if we are only one process holding lock */
    rc = semctl(lock_sem,1,GETVAL);
    if (rc == -1)
    {
	perror("semctl");
	exit(EX_OSERR);
    }
    if (rc == 1)
    {
	/*
	printf("destroying shmem()+sem()\n");
	*/
	/* safe to destroy */
	if (
	     (semctl(lock_sem,0,IPC_RMID) < 0) ||
	     (shmctl(lock_shm,IPC_RMID,NULL) < 0) ||
	     (unlink(key_string) < 0) )
	    rc++;/* ignore cleanup errors */
    }
}
#endif
/********************************************************************/
/********************************************************************/
/********************************************************************/
