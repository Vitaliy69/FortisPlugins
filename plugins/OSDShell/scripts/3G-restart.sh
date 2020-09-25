#!/bin/bash

if [ "$(pidof pppd )" ]
then
        echo "Stoping 3G modem..."
        killall pppd
fi
        sleep 8
        echo "Starting 3G modem..."
        /etc/init.d/ppp_start start&
