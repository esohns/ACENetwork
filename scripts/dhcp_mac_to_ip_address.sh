#!/bin/sh

if [ -z "$1" ]; then
 echo "Usage: $0 mac-address" >&2
 exit 1
fi

CONFIGURATION_FILE=/etc/dhcp/dhclient.conf
[ ! -r "$CONFIGURATION_FILE" ] && echo "invalid dhclient configuration file (was: \"$CONFIGURATION_FILE\"), aborting" && exit 1

PORT=$(grep 'omapi port ' $CONFIGURATION_FILE                     | cut -d' ' -f3 | sed -e 's/;//')
KEY=$(grep 'omapi key '   $CONFIGURATION_FILE                     | cut -d' ' -f3 | sed -e 's/;//')
SECRET=$(grep 'secret '   $CONFIGURATION_FILE | sed -e 's/\s\+//' | cut -d' ' -f2 | sed -e 's/;//')

#key $KEY "$SECRET"
#set hardware-address = $1
#set hardware-type = 1
#set interface = "wlp3s0"
omshell << EOF
server localhost
port $PORT
connect
new lease
set ip-address = 192.168.1.2
open
EOF

#(
#    echo server localhost
#    echo port $PORT
#    echo key $KEY $SECRET
#    echo connect
#    echo new lease
#    echo set fixed-address = 192.168.178.28
#    echo set hardware-address = $1
#    echo open
#) | omshell
#) | omshell | grep '^ip-address ='
