#!/bin/bash

cd ./ChannelStats
make install
cd ../EMURestarter
make install
cd ../KeyChannel
make install
cd ../OSDAlphaScreen
make install
cd ../OSDClock
make install
cd ../OSDECMInfo
make install
cd ../OSDScreenshot
make install
cd ../OSDShell
make install
cd ../../release
mkdir -p ~/FortisPlugins-bin
cp ChannelStats.tar.gz ~/FortisPlugins-bin
cp EMURestarter.tar.gz ~/FortisPlugins-bin
cp OSDAlphaScreen.tar.gz ~/FortisPlugins-bin
cp OSDClock.tar.gz ~/FortisPlugins-bin
cp OSDECMInfo.tar.gz ~/FortisPlugins-bin
cp OSDScreenshot.tar.gz ~/FortisPlugins-bin
cp OSDShell.tar.gz ~/FortisPlugins-bin
cd ../plugins/ChannelStats
make clean
make uninstall
cd ../EMURestarter
make clean
make uninstall
cd ../KeyChannel
make clean
make uninstall
cd ../OSDAlphaScreen
make clean
make uninstall
cd ../OSDClock
make clean
make uninstall
cd ../OSDECMInfo
make clean
make uninstall
cd ../OSDScreenshot
make clean
make uninstall
cd ../OSDShell
make clean
make uninstall
cd ../..
rm -Rf ./release
