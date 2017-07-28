#
# SCons client commands build tester
#
# Version 1.0
# 03-Sep-2009
#

from SCons.Script import ARGUMENTS

def checkForBuildingClients(conf):
    """Check command line arguments if user requested to not build client commands."""
    conf.Message("Checking if we are building client commands... ")
    buildlamer=ARGUMENTS.get('without-clients', 0)
    try:
         buildlamer2=int(buildlamer)
    except ValueError:
         buildlamer2=None
    if buildlamer2 == 1 or str(buildlamer).lower() == 'yes':
        conf.Result(0)
        return False
    else:
        conf.Result(1)
        return True
