#
# Scons variable importer
#
# Version 1.0
# 16-Jun-2009
#
import os

def _setvariable(env,name,value):
   """Sets SCons variable.
   
   Sets Scons variable name in environment env to value

   """
   kw={}
   kw[name]=value
   env.Replace(**kw)

def _setenvvariable(env,name,value):
   """Sets SCons environment variable.
   
   Sets Scons environment variable name in environment env to value.
   This variable will be used as environment variable for
   external jobs started by SCons.

   """
   env['ENV'][name]=value

def importVariable(env,varlist=None,target=None,prefix=None):
   """Imports variables from OS to SCons environment.

   Imports environment variables from Operation System into SCons
   variable.

   keyword arguments:
   env -- SCons environment to be imported into
   varlist -- list or environment variable name to be imported
   target -- output variable names (optional)
   prefix -- import all variables starting with this prefix

   """
   _importcore(env,varlist,target,prefix,_setvariable)

def importEnvironment(env,varlist=None,target=None,prefix=None):
   """Imports variables from OS to SCons environment.

   Imports environment variables from Operation System into SCons
   variable.

   keyword arguments:
   env -- SCons environment to be imported into
   varlist -- list or environment variable name to be imported
   target -- output variable names (optional)
   prefix -- import all variables starting with this prefix

   """
   _importcore(env,varlist,target,prefix,_setenvvariable)

def _importcore(env,varlist,targets,prefix,setter):
   """Imports env. variables from OS using setter.

   keyword arguments:
   env -- SCons environment to be imported into
   varlist -- list or environment variable name to be imported
   targets -- target variable names if you want to import variable
              using different name
   prefix -- import all variables starting with this prefix
   setter -- function for setting variable in env

   """
   if varlist:
       if not isinstance(varlist, list):
           if isinstance(varlist, str):
               varlist=[varlist]
           else:
               raise TypeError,"varlist must be list or string"
       if targets:
           if not isinstance(targets, list):
               if isinstance(targets, str):
                   targets=[targets]
               else:
                   raise TypeError,"targets must be list of string"
       for i in range(0,len(varlist)):
           value=os.environ.get(varlist[i])
	   if value:
               if targets:
	          setter(*[env,targets[i],value])
               else:   
	          setter(*[env,varlist[i],value])
   if prefix:
       if not isinstance(prefix, str):
           raise TypeError,"prefix must be string"
       for i in os.environ.keys():
	   if i.startswith(prefix):
               setter(*[env,i,os.environ.get(i)])
