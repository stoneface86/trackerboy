#!/bin/bash
# script installs all dependencies needed for linux builds/deployment

# rtmidi requires ALSA dev libraries (not provided by vcpkg)
sudo apt-get update
sudo apt-get install libasound2-dev
# acquire linuxdeployqt, put into directory from argument $1
mkdir "$1"
cd "$1"
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage -nv -O linuxdeployqt
chmod a+x linuxdeployqt
