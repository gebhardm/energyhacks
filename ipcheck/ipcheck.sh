#!/bin/sh

# set a catchy subject, but be aware that blanks end it ;-/
SUBJ="External_IP_Address"
# replace by your receiver email address
EMAIL="your.email@domain.toplevel"
# the ip addresses to be compared
ip1=""
ip2=""
# prepare the comparison
read ip1 < ip.txt
ip2=$(curl ident.me)
# end actually execute it
if [ "$ip1" = "$ip2" ]
then
  exit
else
  echo "$ip2" > ip.txt
  echo "$ip2" | mail -s $SUBJ $EMAIL
  exit
fi
