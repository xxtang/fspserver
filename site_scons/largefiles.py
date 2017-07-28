#
# SCons Largefile enablement
#
# Version 1.1
# 01-Oct-2009
#

def enableLargeFiles(conf):
    """Tries to enable 64-bit off_t on linux platform"""
    fseeko=conf.CheckFunc('fseeko')
    if fseeko:
        conf.env.Append(CPPFLAGS = '-DHAVE_FSEEKO')
    offt=conf.CheckTypeSize('off_t','#include <stdio.h>\n#include <sys/types.h>')
    if offt<8 and offt>0:
        flags=conf.env.Dictionary()['CPPFLAGS']
	conf.env.Append(CPPFLAGS='-D_FILE_OFFSET_BITS=64')
        offt=conf.CheckTypeSize('off_t','#include <stdio.h>\n#include <sys/types.h>')
        if offt < 8:
              env.Replace(CPPFLAGS=flags)
    else:
      if offt == 0:
         #set default value to 4
         offt=4
    conf.env.Append(CPPFLAGS = '-DSIZEOF_OFF_T='+str(offt))
    if fseeko and int(offt)>=8:
       conf.env.Append(CPPFLAGS = '-DNATIVE_LARGEFILES')
       rc=True
    else:
       rc=False
    return rc
