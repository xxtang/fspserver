#
# SCons DSSSL processor check
#
# Version 1.1
# 08-Sep-2014
#

import subprocess

def checkDSSSLProcessor(check, name="jade"):
    """Check if DSSSL engine is working. Returns True or False."""
    check.Message("Checking if DSSSL processor "+name+" works... ")
    try:
       echo = subprocess.Popen(['/bin/echo', '""'], stdout = subprocess.PIPE )
       version = subprocess.Popen([name, '-v'], stdin = echo.stdout, stdout= subprocess.PIPE, stderr = subprocess.PIPE )
       stderr = repr(version.communicate()[1])
       if "version" in stderr:
          check.Result(True)
          return True
    except subprocess.CalledProcessError:
       pass
    check.Result(False)
    return False
