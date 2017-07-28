#
# SCons checker for enabling client timeout feature
#
# Version 1.0
# 18-Aug-2009
#

from SCons.Script import ARGUMENTS

def checkForClientTimeout(conf):
    """Check command line arguments if user requested disabling timeouts."""
    conf.Message("Checking if client commands can time out... ")
    timeout=ARGUMENTS.get('disable-timeout', 0)
    try:
         timeout2=int(timeout)
    except ValueError:
         timeout2=None
    if timeout2 == 0 or str(timeout).lower() == 'no':
        conf.env.Append(CPPFLAGS = '-DCLIENT_TIMEOUT')
        conf.Result(1)
    else:
        conf.Result(0)
