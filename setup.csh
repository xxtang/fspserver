
# Sample csh initialization file.

setenv FSP_PORT 2000
setenv FSP_HOST localhost
setenv FSP_DIR  /
setenv FSP_TRACE
setenv FSP_DELAY 3000

alias fcat	\(set noglob\; exec fcatcmd   \!\*\)
alias fstat	\(set noglob\; exec fstatcmd   \!\*\)
alias fcd	setenv FSP_DIR \`\(set noglob\; exec fcdcmd \!\*\)\`
alias fdu	\(set noglob\; exec fducmd    \!\*\)
alias ffind	\(set noglob\; exec ffindcmd  \!\*\)
alias fget	\(set noglob\; exec fgetcmd   \!\*\)
alias fgrab	\(set noglob\; exec fgrabcmd  \!\*\)
alias fhost	'eval `fhostcmd \!*`'
alias fless	\(set noglob\; exec fcatcmd   \!\* \| less\)
alias fls	\(set noglob\; exec flscmd    \!\*\)
alias fmore	\(set noglob\; exec fcatcmd   \!\* \| more\)
alias fpro	\(set noglob\; exec fprocmd   \!\*\)
alias fpwd	echo \$FSP_DIR on \$FSP_HOST port \$FSP_PORT
alias frm	\(set noglob\; exec frmcmd    \!\*\)
alias frmdir	\(set noglob\; exec frmdircmd \!\*\)
alias fsetup	'eval `fsetupcmd \!*`'
alias fmv	\(set noglob\; exec fmvcmd    \!\*\)
alias ftouch	"touch \!:1;fput \!:1;rm \!:1"
