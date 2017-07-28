#
# SCons user-supplied sysconfdir configuration
#
# Version 1.0
# 27-Aug-2009
#

from SCons.Script import ARGUMENTS

def checkForUserSysconfdir(conf,prefix):
    """Returns sysconfdir specified on command line or prefix/etc if none is supplied."""
    conf.Message("Checking for user supplied sysconfdir... ")
    lp = ARGUMENTS.get('sysconfdir', 0)

    if lp:
       conf.Result(1)
    else:
       conf.Result(0)
       lp=prefix+'/etc'
       
    conf.env.Append(CPPFLAGS = '-DSYSCONFDIR=\\"'+lp+'\\"')
    return lp
