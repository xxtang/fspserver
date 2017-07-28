#
# SCons C language related tests
#
# Version 1.2
# 24-Aug-2009
#

def checkForVariable(conf,variable,include):
    """Checks if variable is defined in given include statements."""
    conf.Message("Checking if variable %s is defined... " % variable)
    rc = conf.TryCompile("""
%s
void dummy(void);
void dummy(void) { %s = 0; }
"""% (include,variable),'.c')
    conf.Result(rc)
    return rc
   
def getVariableSize(conf,var):
    """Returns variable size in bytes"""
    conf.Message("Checking for size of "+var+"... ")
    rc = conf.TryCompile("""
#include <stdio.h>
#include <sys/types.h>    
      
main ()
{
 if ((%s *) 0)
   return 0;
 if (sizeof (%s))
   return 0;
 ;
 return 0;
}
""" % (var,var),'.c')
    if rc:
	rc,result = conf.TryRun('''
#include <stdio.h>
#include <sys/types.h>	

main ()
{
    printf("%%d",sizeof(%s));
    return 0;
}''' % var,'.c')
	if rc:
	    rc=result
    conf.Result(rc)
    return rc
