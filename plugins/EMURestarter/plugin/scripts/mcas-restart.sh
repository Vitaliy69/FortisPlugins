#!/bin/bash

if [ "$(pidof mcas)" ]
then
    killall mcas
fi

/var/bin/mcas&
