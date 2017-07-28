#! /usr/bin/python
import sys

if __name__ == '__main__':
   if len(sys.argv)<3:
       print sys.argv[0],'<filename> <filesize (in GB)>'
   else:
       GB=1024*1024*1024
       f=open(sys.argv[1],'w')
       f.seek(long(float(sys.argv[2])*GB))
       f.write('!')
       f.close();
