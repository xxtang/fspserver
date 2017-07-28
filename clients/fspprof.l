%option nounput
%option case-insensitive
%option never-interactive
%option noyylineno

%{
#include <string.h>
#include "fhost.h"

static struct fsp_host *host=NULL;
%}
DIGIT [0-9]
NUMBER {DIGIT}+
WHITECHAR [ \t\n\r]
OPTWHITESPACE {WHITECHAR}*
WHITESPACE {WHITECHAR}+
COMMENT #.*$
IPADDRESS {NUMBER}\.{NUMBER}\.{NUMBER}\.{NUMBER}
HOSTNAME [[:alpha:]][\-._[:alnum:]]+
	/* stavy */
%s shostname
%s shost
%s sport
%s salias
%s slport
%s sdir
%s sldir
%s sdelay
%s spassword
%s stimeout
%%
{COMMENT}     /* ignore comments */
^{WHITESPACE} /* ignore whitespace at begining of line */
^{OPTWHITESPACE}(host|machine){WHITESPACE} {
	add_host(host);
	host=NULL;
	BEGIN(shostname);
	}
<shostname>{HOSTNAME} {
			host=init_host();
			/* printf("host %s!\n",yytext); */
			host->hostname=strdup(yytext);
			BEGIN(shost);
		      }
<shostname>{IPADDRESS} {
			host=init_host();
			/* printf("ihost %s!\n",yytext); */
			host->hostaddr=strdup(yytext);
			BEGIN(shost);
			}
<shostname>.|\n fprintf(stderr,"invalid hostname `%s`\n",yytext);BEGIN(INITIAL);
<shost>(fsp{WHITESPACE})?port   BEGIN(sport);
<sport>{WHITESPACE}
<sport>{NUMBER} host->port=atoi(yytext);BEGIN(shost);
<sport>.|\n fprintf(stderr,"invalid port `%s`\n",yytext);BEGIN(shost);
<shost>alias{WHITESPACE}   BEGIN(salias);
<salias>{HOSTNAME}         add_host_alias(host,yytext);BEGIN(shost);
<shost>local{WHITESPACE}port  BEGIN(slport);
<slport>{WHITESPACE}
<slport>{NUMBER}  host->local_port=atoi(yytext);BEGIN(shost);
<shost>(fsp{WHITESPACE})?directory{WHITESPACE} BEGIN(sdir);
<sdir>\/[[:graph:]]*  host->dir=strdup(yytext);BEGIN(shost);
<sdir>.|\n  fprintf(stderr,"invalid directory `%s`\n",yytext);BEGIN(shost);
<shost>local{WHITESPACE}directory{WHITESPACE} BEGIN(sldir);
<sldir>[[:graph:]]*  host->local_dir=strdup(yytext);BEGIN(shost);
<sldir>.|\n fprintf(stderr,"invalid local directory `%s`\n",yytext);BEGIN(shost);
<shost>delay BEGIN(sdelay);
<sdelay>{WHITESPACE}
<sdelay>{NUMBER} host->delay=atoi(yytext);BEGIN(shost);
<sdelay>.|\n fprintf(stderr,"invalid delay `%s`\n",yytext);BEGIN(shost);
<shost>timeout BEGIN(stimeout);
<stimeout>{WHITESPACE}
<stimeout>{NUMBER} host->timeout=atoi(yytext);BEGIN(shost);
<stimeout>.|\n fprintf(stderr,"invalid timeout `%s`\n",yytext);BEGIN(shost);
<shost>trace{WHITESPACE}on host->trace=1;
<shost>trace{WHITESPACE}off host->trace=0;
<shost>password{WHITESPACE} BEGIN(spassword);
<spassword>[[:graph:]]+ host->password=strdup(yytext);BEGIN(shost);
<shost><<EOF>> add_host(host);host=NULL;yyterminate();

{WHITECHAR} /* ignore */
.  fprintf(stderr," %s",yytext);/* no default rule */
%%
