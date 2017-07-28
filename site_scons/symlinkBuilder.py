#
# Symlink SCons builder
#
# Version 1.0
# 02-Aug-2009
#

def Symlink(target, source, env=None):
   """Create symlink target pointing to source.
   
   This builder creates symlinks named target pointing to source.
   Target is removed if exists.

   Keyword arguments:
   target -- list of symlink targets
   source -- list of files to be compressed
   env -- SCons environment (not used)
   """
   import os
   if not isinstance(target, list):
       raise TypeError,"target must be list"
   elif not isinstance(source, list):
       raise TypeError,"source must be list"
   if len(target) != len(source):
       raise ValueError,"target and source list must have same size"    
   for i in range(0,len(target)):
       try:
          os.unlink(str(target[i]))
       except OSError:
          pass   
       os.symlink(str(source[i]),str(target[i]))
   return None
