    /*********************************************************************\
    *  Copyright (c) 2003 by Radim Kolar (hsn@cybermail.net)              *
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include <ctype.h>
#include <netdb.h>
#include "server_def.h"
#include "s_extern.h"
#include "my-string.h"

/* Function for IPrange structure */

IPrange *iptab=NULL;

#define skip_whitespace(x) do {while (*(x)&&isspace(*(x))) (x)++;} while (0)

/* check if inet_number matches entry in IPrange table */
/* return pointer to specified message                 */

static char *check_ip (unsigned long inet_num, const IPrange * iprange)
{
  unsigned int j;
  unsigned char val[4];

  inet_num = ntohl(inet_num);

  val[0] = (inet_num & 0x000000ff)      ;
  val[1] = (inet_num & 0x0000ff00) >>  8;
  val[2] = (inet_num & 0x00ff0000) >> 16;
  val[3] = (inet_num & 0xff000000) >> 24;

    for (j = 0; j < 4; j++)
      if (iprange->lo[j] > val[j] || val[j] > iprange->hi[j]) return NULL;

  return iprange->text;
}


const char *check_ip_table (unsigned long inet_num,IPrange *table)
{
    char *res;

    while(table)
    {
	if(!table->text) return NULL;  /* EOT! */
	res=check_ip(inet_num,table);
	if(res) return res;
	table++;
    }
    return NULL;
}

void free_ip_table (IPrange *table)
{
    IPrange *head=table;
    /* free strings */
    while(table++)
    {
	if(!table->text) break;
	free(table->text);
    }
    free(head);
}

/* IP parsing fun */
/* parse the text string as an integer and return a value between 0x00 and 0xff
   if there is any error in forming the value (e.g., the text string
   isn't an integer, or the integer value is too large) then make the
   text string NULL */
static unsigned char parse_ipcomponentnum (const char * * textp)
{
  unsigned long val = 0;

  if (!isdigit(**textp)) {
    *textp = 0;
  } else
    do {
      val = 10 * val + (**textp - '0');
      (*textp)++;
    } while (isdigit(**textp));

  if (val > 0xff) {
    val = 0;
    *textp = 0;
  }

  return val;
}

/* parse a whole field of a numerical IP address; it can be one of:
   integer		>> fixed value
   integer '-' integer	>> range of values
   '*'			>> same as 0-255
   */
static const char *parse_ipcomponent (const char * text, unsigned char * lo,
				      unsigned char * hi)
{
  if (*text == '*') {
    *lo = 0x00;
    *hi = 0xff;
    return (text + 1);
  }

  *lo = parse_ipcomponentnum(&text);
  if (!text) return 0;

  if (*text == '-') {
    text++;
    *hi = parse_ipcomponentnum(&text);
  } else *hi = *lo;

  return text;
}

static IPrange *parse_ipnumber (const char * text)
{
  IPrange *reply;
  int i;

  reply = (IPrange *)malloc(sizeof(IPrange));

  for (i = 3; i >= 0 && !isspace(*text); i--) {
    if (i < 3) {
      if (*text != '.') return 0;
      else text++;
    }
    text = parse_ipcomponent(text, &reply->lo[i], &reply->hi[i]);
    if (!text) {
      free((char *)reply);
      return 0;
    }
  }

  /* fill in the gaps in the case that the loop terminated due to
     the occurrence of white-space */
  for (; i >= 0; i--) {
    reply->lo[i] = 0x00;
    reply->hi[i] = 0xff;
  }

  return reply;
}

static IPrange *parse_hostname (const char * text, unsigned int len)
{
  IPrange *reply;
  struct hostent *hostaddr;
  unsigned long inet_num;
  char *hostname;

  hostname = malloc(len + 1);
  strncpy(hostname, text, len);
  hostname[len] = 0;

  hostaddr = gethostbyname(hostname);
  free(hostname);

  if (!hostaddr) return 0;

  reply = (IPrange *)malloc(sizeof(IPrange));
  inet_num = ((struct in_addr *)(hostaddr->h_addr))->s_addr;
  reply->lo[0] = reply->hi[0] = (inet_num & 0x000000ff)      ;
  reply->lo[1] = reply->hi[1] = (inet_num & 0x0000ff00) >>  8;
  reply->lo[2] = reply->hi[2] = (inet_num & 0x00ff0000) >> 16;
  reply->lo[3] = reply->hi[3] = (inet_num & 0xff000000) >> 24;

  return reply;
}

/* parse a single line for the IP hosts file                 */
/* returns IPrange structure,first letter in message is type */

/*
# syntax of ipline is: host_mask host_type [message]
# host_type is one of D, I, or N :
#   I hosts are ignored
#   N hosts are treated as normal
#   D hosts will receive the error string message given as the third parameter
#   128.4-8.*.* -- (* acts as the range 0 - 255)
*/

static IPrange *parse_ipline (const char * text)
{
  IPrange *reply;
  char type = 0;
  const char *message = 0;
  size_t messlen = 0, addresslen;

  /* skip the leading white-space */
  skip_whitespace(text);
  /* if the line is commented or empty, ignore it */
  if (!*text || *text == '#') return 0;

  /* load hostname range (can not have spaces inside) */
  message = text;
  while (*message && !isspace(*message)) message++;
  addresslen = message - text;

  /* find the first non-space character after the address - this
     identifies the type of host this is */
  skip_whitespace(message);

  if (!*message || *message == '#') {
    fprintf(stderr, "No host type specified in config file:\n\t%s\n", text);
    /* if a host name is mentioned by itself, then treat it as ignored
       or normal depending on the value of priv_mode */
    return 0;
  }

  /* the first character after the host name is the type of host */
  type = *message;

  /* skip over the white space trailing the type - the start of the
     associated message */
  message++; /* remember skip_whitespace() is a macro... */
  skip_whitespace(message);

  /* `remove' the trailing white-space from the message */
  messlen = strlen(message);
  while (messlen > 0 && isspace(message[messlen-1])) messlen--;

  /* if the first character of the address is numerical or '*' then parse
     as a numerical address, otherwise we do a host lookup on the name. */
  if (*text == '*' || isdigit(*text))
    reply = parse_ipnumber(text);
  else
    reply = parse_hostname(text, addresslen);

  if (!reply) {
    fprintf(stderr, "Badly formed address in config file:\n\t%s\n", text);
    return 0;
  }

  /* allocate a string to hold the message */
  reply->text = malloc(1 + messlen + 1); /* type + text + '\0' */
  if(!reply->text)
  {
      free(reply);
      return 0;
  }
  reply->text[0] = type;
  strncpy(&reply->text[1], message, messlen);
  reply->text[1 + messlen] = '\0';

  return reply;
}


void add_ipline (const char * text, IPrange ** table)
{
      IPrange *nl;
      IPrange *newtab;
      int i;

      nl = parse_ipline(text);
      if(!nl) return;
      /* add a new record to table */
      if(*table==NULL)
      {
	  *table=malloc(sizeof(IPrange)*2);
	  if(!*table) return;
	  memcpy(*table,nl,sizeof(IPrange));
	  free(nl);
	  ((*table)+1)->text=NULL;
	  return;
      }
      /* count records in IPrange table */
      i=0;
      while( ((*table)+i++)->text!=NULL);
      newtab=realloc(table,sizeof (IPrange) * (i+1));
      if(!newtab) return;
      *table=newtab;
      memcpy( (*table)+i,nl,sizeof(IPrange));
      free(nl);
      ((*table)+i+1)->text=NULL;
}
/****************************************************************************
 * Write out the IP table in the .IPTAB_DUMP file.
 ****************************************************************************/

void dump_iptab (IPrange *table,FILE * fp)
{

  if(fp==NULL)
  {
      if(dbug)
	  fp=stdout;
      else
	  return;
  }

  while(table) {
           if(!table->text) break;
    fprintf(fp, "%d-%d.%d-%d.%d-%d.%d-%d  %c  %s\n",
	    table->lo[3], table->hi[3],
	    table->lo[2], table->hi[2],
	    table->lo[1], table->hi[1],
	    table->lo[0], table->hi[0],
	    table->text[0],&table->text[1]);
           table++;
  }
}
