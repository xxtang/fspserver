%option nounput
%option never-interactive
%option noyylineno

%{
#include <string.h>
#include "fhost.h"

static struct fsp_host *host=NULL;
/* The standard for the format for fspsites file is:

        hostname port alias root-directory # comment
*/
%}
DIGIT [0-9]
NUMBER {DIGIT}+
WHITECHAR [ \t]
EOL [\r\n]+
OPTWHITESPACE {WHITECHAR}*
WHITESPACE {WHITECHAR}+
COMMENT #[^\r\n]*
IPADDRESS {NUMBER}\.{NUMBER}\.{NUMBER}\.{NUMBER}
HOSTNAME [[:alpha:]][\-._[:alnum:]]+
 /* stavy */
%s sport
%s salias
%s sdir
%%
{COMMENT}		     /* just ignore comments */
{WHITESPACE}		     /* eat me too */
<INITIAL>{HOSTNAME}  {
			host=init_host();
			/* printf("host %s!\n",yytext); */
			host->hostname=strdup(yytext);
			BEGIN(sport);
		        }
<INITIAL>{IPADDRESS} {
			host=init_host();
			/* printf("ihost %s!\n",yytext); */
			host->hostaddr=strdup(yytext);
			BEGIN(sport);
			}
<sport>{NUMBER} 	host->port=atoi(yytext);BEGIN(salias);
<salias>{HOSTNAME}      add_host_alias(host,yytext);
<salias>\/[[:graph:]]*  host->dir=strdup(yytext);BEGIN(INITIAL);
{EOL}			add_host(host);host=NULL;BEGIN(INITIAL);
<<EOF>>			add_host(host);host=NULL;BEGIN(INITIAL);yyterminate();
