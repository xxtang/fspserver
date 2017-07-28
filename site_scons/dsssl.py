#
# SCons Locate DSSSL docbook stylesheets
#
# Version 1.2
# 09-Sep-2014
#

import os
import stat
from SCons.Script import ARGUMENTS

def findDocbookStylesheets(check):
    """Return path to Docbook stylesheets or None."""
    directories = [
                    "/usr/local/share/sgml/docbook/dsssl/modular",
                    "/usr/share/sgml/stylesheets/dsssl/docbook"
    ]
    customdsssl = ARGUMENTS.get('dsssl', None)
    if customdsssl: directories.insert(0, customdsssl)
    files = [ "html/docbook.dsl", "catalog", "common/dbcommon.dsl" ]
    check.Message("Locating Docbook DSSSL Stylesheets... ")
    for d in directories:
        try:
           st = os.stat(d)
           if not stat.S_ISDIR(st.st_mode):
               continue
           for f in files:
               st = os.stat(d+os.sep+f)
               if not stat.S_ISREG(st.st_mode):
                   raise OSError
           check.Result(d)
           return d
        except OSError:
               continue
    check.Result(False)
    return None
