#!/bin/sh
#  install.sh
#  LeitzIcon
#
#  Created by Olga Fedorechko on 16.11.14.
#  Copyright (c) 2014, Esselte IPR AB. All rights reserved.


pwd=`pwd`
filtersDirectory="/usr/lib/cups/filter"
PPDsDirectory="/usr/share/cups/model/Leitz"
echo $pwd

cd "$pwd"
gcc -std=gnu99 -o raster2printer raster_c_functions.c raster2printer.c -lcups -lcupsimage

if [ -f raster2printer ];
then
    echo "raster filter compiled ..."
    if [ -x $filtersDirectory ];
    then
        cp raster2printer $filtersDirectory
        cd $filtersDirectory
        chown root:root raster2printer
        chmod 0755 raster2printer
    fi
fi

mkdir -p "$PPDsDirectory"

if [ -x "$PPDsDirectory" ];
then
    cp "$pwd"/PPD/LeitzIcon.ppd.gz $PPDsDirectory
    cp "$pwd"/Resources/LeitzPrintIcon.icns $PPDsDirectory
    cd "$PPDsDirectory"
    chmod 0644 LeitzIcon.ppd.gz
    chown root:root LeitzIcon.ppd.gz
    chmod 0644 LeitzPrintIcon.icns
    chown root:root LeitzPrintIcon.icns
    echo "installed resources..."
fi

exit 0