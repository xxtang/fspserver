#
# SCons user-supplied docdir configuration
#
# Version 1.0
# 03-Sep-2009
#

from SCons.Script import ARGUMENTS
import os.path

def checkForUserDocdir(conf,olddocdir=None):
    """Returns docdir specified on command line or olddocdir if none is supplied."""
    conf.Message("Checking for user supplied docdir... ")
    lp = ARGUMENTS.get('docdir', 0)
    if lp:
       conf.Result(1)
       return lp
    else:
       conf.Result(0)
       return olddocdir
