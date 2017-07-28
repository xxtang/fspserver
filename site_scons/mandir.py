#
# SCons user-supplied mandir configuration
#
# Version 1.0
# 27-Aug-2009
#

from SCons.Script import ARGUMENTS
import os.path

def checkForUserMandir(conf,oldmandir=None):
    """Returns mandir specified on command line or oldmandir if none is supplied."""
    conf.Message("Checking for user supplied mandir... ")
    lp = ARGUMENTS.get('mandir', 0)
    if lp:
       conf.Result(1)
       return lp
    else:
       conf.Result(0)
       return oldmandir

def autodetectMandir(conf,prefix):
    """Try to autodetect where to put man pages."""
    SEARCH = [ prefix+'/share/man', prefix+'/man' ]
    conf.Message("Checking where to install man pages... ")
    for d in SEARCH:
        if os.path.isdir(d):
            conf.Result(d)
            return d
    d = prefix + '/man'
    conf.Result(d)
    return d
