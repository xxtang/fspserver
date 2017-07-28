/*********************************************************************\
 *  Copyright (c) 2003 by Radim Kolar (hsn@cybermail.net)              *
 *                                                                     *
 *  You may copy or modify this file in any manner you wish, provided  *
 *  that this notice is always included, and that you hold the author  *
 *  harmless for any loss or damage resulting from the installation or *
 *  use of this software.                                              *
 \*********************************************************************/

#include "tweak.h"
#include <stdio.h>
#include "server_def.h"
#include "s_extern.h"
#include "my-string.h"

extern char REC_PASSWD_T[128];


/* loads a password from file in current directory */
static char *load_password (const char * file)
{
	/* load password */
	FILE *f;
	char fsp_passwd[80];

	f=fopen(file,"r");
	if(f) {
		if(fscanf(f, "%79s", fsp_passwd) != 1) {
			if(dbug) printf("load_pass: no password found in file\n");
			fclose(f);
			return NULL;
		}
	} else
		return NULL;
	fclose(f);
	return strdup(fsp_passwd);
}

/* save a password to file in current directory */
static void save_password (const char *file,const char *password)
{
	/* load password */
	FILE *f;

	f=fopen(file,"w");
	if(f) {
		fprintf(f,"%s\n",password);
		fclose(f);
	}
}

/* save access rights to filesystem */
void save_access_rights (DIRINFO *di)
{
	/* step 1 - unlink everything */
	unlink(FSP_NOGET);
	unlink(FSP_DEL);
	unlink(FSP_ADD);
	unlink(FSP_MKDIR);
	unlink(FSP_RENAME);
	unlink(FSP_NOLIST);
	/*unlink(FSP_PASSWORD);*/
	unlink(FSP_OWNER);
	unlink(FSP_OWNERPASSWORD);
	unlink(FSP_DIRLISTING);

	if(!use_access_files) return;

	/* step2 save flagfiles */
	if(! (di->protection & DIR_GET) )	touch(FSP_NOGET);
	if(di->protection & DIR_DEL)		touch(FSP_DEL);
	if(di->protection & DIR_ADD)		touch(FSP_ADD);
	if(di->protection & DIR_MKDIR)	touch(FSP_MKDIR);
	if(di->protection & DIR_RENAME) 	touch(FSP_RENAME);
	if(! (di->protection& DIR_LIST))   	touch(FSP_NOLIST);

	/* step3 pazzwordz 
	//if(di->public_password) save_password(FSP_PASSWORD,di->public_password);*/
	if(di->owner_password) save_password(FSP_OWNERPASSWORD,di->owner_password);

	/* save Owner ACL */
	if(di->owner)
	{
		FILE *f;
		f=fopen(FSP_OWNER,"w");
		if(f)
		{
			dump_iptab(di->owner,f);
			fclose(f);
		}
	}
}

/* called with current directory == tested directory */
/* load access rights for validated directory */
void load_access_rights (DIRINFO *di)
{
	
	if(dbug) fprintf(stderr,"load access rights\n");/*Add xxfan*/
	struct stat sf;
	FILE *f;
	char owner_line[256];

	di->owner=NULL;
	di->owner_password=NULL;
	/*di->public_password=NULL;*/
	di->public_password=REC_PASSWD_T;
	di->protection=DIR_LIST|DIR_GET;
	di->readme=NULL;

	/* load directory readme file */
	if(!FSP_STAT(readme_file,&sf))
	{
		f=fopen(readme_file,"r");
		if(f)
		{
			unsigned int s;
			/* max readme file size is
			 * packetsize - pro_bytes (now 1) - 1 (for /0)
			 */
			s=min(packetsize-PRO_BYTES-1,sf.st_size);
			di->readme=calloc(1,s+1); /* allocate also space for ending /0 */
			if(di->readme)
			{
				fread(di->readme,s,1,f);
				di->protection|=DIR_README;
			}
			fclose(f);
		}
	}

	if(!use_access_files) return;

	/* LOAD ACCESS RIGHTS FROM FILES IN CURRENT DIRECTORY */
	if(fexist(FSP_NOGET))   di->protection^=DIR_GET;
	if(fexist(FSP_DEL))     di->protection|=DIR_DEL;
	if(fexist(FSP_ADD))     di->protection|=DIR_ADD;
	if(fexist(FSP_MKDIR))   di->protection|=DIR_MKDIR;
	if(fexist(FSP_RENAME))  di->protection|=DIR_RENAME;
	if(fexist(FSP_NOLIST))  di->protection^=DIR_LIST;

	if(!FSP_STAT(FSP_PASSWORD,&sf))
	{
		if(sf.st_size>0)
		{
			di->public_password=load_password(FSP_PASSWORD);
		}
	}
	/* load owner info */
	f=fopen(FSP_OWNER,"r");
	if(!f)
		return;

	while(fscanf(f, "%255[^\n\r]", owner_line) == 1)
	{
		add_ipline(owner_line,&di->owner);
	}
	fclose(f);
	/* .. and password */
	if(!FSP_STAT(FSP_OWNERPASSWORD,&sf))
	{
		if(sf.st_size>0)
			di->owner_password=load_password(FSP_OWNERPASSWORD);
	}
}

/* does caller has enough security rights? */
/* if rights has DIR_PRIV set than caller is restristed and need also
 * at least 'N' record to get it, if he has DIR_README, we will send him
 * a datagram with error reply */

const char * require_access_rights (const DIRINFO *di,unsigned char rights,unsigned long ip_addr, const char * passwd)
{
	const char *acc=NULL;

	/* check if we has record in an owner tabl. */
	if(di->owner)
	{
		/* check ip access */
		acc=check_ip_table(ip_addr,di->owner);
	}

	if(acc!=NULL)
	{
		if(acc[0]=='I' || acc[0]=='D')
			/* bazmeg IP! */
			return acc;
		if(acc[0]=='O') /* possible owner */
		{
			if(di->owner_password==NULL)
			{
				if(permit_passwordless_owners)
					return acc; /* passwordless owner */
			} else
			{
				/* do we have a password? */
				if(passwd)
				{
					/* compare passwords */
					if(!strcmp(passwd,di->owner_password))
						return acc; /* owner with good password */
				} else
					if(rights & DIR_OWNER)
						return "DPassword required for owner";
			}
		}
	}

	if(rights & DIR_OWNER)
		return "DNot an owner";
	/* is password needed for public access? */
	if(di->public_password)
	{
		fprintf(stderr,"di->public_password-%s,passwd-%s\n",di->public_password,passwd);
		if(passwd==NULL)
			return "DYou need a password";
		else
			if(strcmp(di->public_password,passwd))
			{
				return "DInvalid password";
			}
	}

	/* now check public access rights */
	if( (rights & di->protection) ||
			(rights==0)
	  )
		return "NWelcome on board captain!";
	else
		return "DPermission denied";
}
