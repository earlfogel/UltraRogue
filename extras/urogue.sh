#!/bin/sh

# set various options
#export SROGUEOPTS=cutcorners,difficulty=easy

# If we have a saved game, resume it, otherwise start a new one.
if [ -e ~/rogue.save ] ; then
    urogue ~/rogue.save
else
    urogue $@
fi
