#!/bin/csh -f

# Set this to where this program, and convdate reside so they can be invoked.
set MY_DIR = /usr/fsp/bin

# Set this to where you want the log files stored.  If you don't want
# any log files, set this to /dev/null
set LOG_DIR = /usr/fsp/logs

# Set this to point to the top level of your FSP export directory
set FSP_HOME = /usr/fsp/data

# auto_del version 1.0

# A c-shell script that uses convdate, and walks all directories under
# your FSP home directory in order to find files that are out dated.
# It allows complete control of how long to leave files alone via 2
# files, .FSP_EXPIRE and .FSP_TIMEOUT.  If neither of these two files
# exist, the directory is ignored, if either of them exist, every file
# in the directory is tested to see if it should be expired or timed
# out.  Each of the two files contains one line containing an integer.
# This integer is the number of days after which a file should be
# expired or timed out.   The .FSP_EXPIRE causes the script to check
# the current date against the last access (includes reads) time and
# if it's greater, remove it.  The .FSP_TIMEOUT checks the last
# modified time and compares it to the date and if it has been around
# too long, to remove it.

# This simple script written by Joseph Traub (jtraub@cs.cmu.edu)
# Feel free to use it, hack it, have sex with it, or whatever.
# Author disclaims any responsibility whatsoever for any acts performed
# with this script or any derivitive thereof.


# if we are called with no arguments start at the top level of the export dir.
# otherwise start at the directory we're called with.
if ( ${#argv} == 1 ) then
  cd $1
else
  cd $FSP_HOME
endif

# Look for the two special files that mark this directory as a victim for
# this scripts tender mercies.
if ( -e .FSP_EXPIRE || -e .FSP_TIMEOUT) then
  set can_remove = 1
else
  set can_remove = 0
endif

# Get the time limit for auto-expiration.  The file .FSP_EXPIRE should
# contain one line which has an integer number of days.
if (-e .FSP_EXPIRE) then
  set expire_time = `cat .FSP_EXPIRE`
else
  set expire_time = 0
endif

# Get the time limit for auto-expiration.  The file .FSP_TIMEOUT should
# contain one line which has an integer number of days.
if (-e .FSP_TIMEOUT) then
  set time_out = `cat .FSP_TIMEOUT`
else
  set time_out = 0
endif

# I include .* here as a hack to keep it from printing errors when it
# doesn't find any normal files.
foreach i (* .*)
  if ( $i !~ .* ) then
    if ( -d $i ) then
       $MY_DIR/auto_del $i
    else
       if ( $can_remove ) then
         set removed = 0
         set atime = `$MY_DIR/convdate -a $i`
         set mtime = `$MY_DIR/convdate -m $i`
         set d = `date`
         set date = `$MY_DIR/convdate -d $d`
         set file_age = 0
         @ file_age = $date - $mtime
         set last_read = 0
         @ last_read = $date - $atime
         if ( $time_out ) then
            if ( $time_out < $file_age ) then
               \rm -rf $i >& /dev/null
               echo "[$d] timedout $i" >> $LOG_DIR/auto_del.log
               set removed = 1
            endif
         endif
         if ( $expire_time && ! $removed ) then
           if ( $expire_time < $last_read ) then
             \rm -rf $i >& /dev/null
             echo "[$d] expired $i" >> $LOG_DIR/auto_del.log
           endif
         endif
       endif
    endif
  endif
end
