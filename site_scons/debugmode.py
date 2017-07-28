#
# SCons debug mode build tester
#
# Version 1.2
# 13-Sep-2009
#

from SCons.Script import ARGUMENTS

def checkForDebugBuild(conf):
    """Check command line arguments if user requested debug mode build."""
    conf.Message("Checking if we are building with extra debug code... ")
    buildlamer=ARGUMENTS.get('enable-debug', 0)
    try:
         buildlamer2=int(buildlamer)
    except ValueError:
         buildlamer2=None
    if buildlamer2 == 0 or str(buildlamer).lower() == 'no':
        conf.Result(0)
        return False
    else:
        conf.env.Append(CPPFLAGS = '-DDEBUG')
        conf.Result(1)
        return True
