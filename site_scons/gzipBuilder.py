#
# GZip SCons builder
#
# Version 1.0
# 16-Jun-2009
#

def GZip(target, source, env=None):
   """Compress files with gzip using default compression level.

   Compress source files into target files using gzip
   compression. No checking on datestamps of possible existing
   target file is done, its always overwritten.

   Keyword arguments:
   target -- list of compressed files to made
   source -- list of files to be compressed
   env -- SCons environment (not used)
   """
   import gzip
   if not isinstance(target, list):
       raise TypeError,"target must be list"
   elif not isinstance(source, list):
       raise TypeError,"source must be list"
   for i in range(0,len(target)):
       inpf=str(source[i])
       outf=str(target[i])
       out=gzip.open(outf,"wb")
       inp=file(inpf,"rb")
       out.write(inp.read())
       out.close()
       inp.close()
   return None
