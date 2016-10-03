#!/bin/sh

SUBJ="External IP Address"
EMAIL="your.email@domain.toplevel"

ip1=""
ip2=""

read ip1 < ip.txt
ip2=$(curl ident.me)

if [ "$ip1" = "$ip2" ]
then
  exit
else
  echo "$ip2" > ip.txt
  echo "$ip2" | mail -s $SUBJ $EMAIL
  exit
fi
