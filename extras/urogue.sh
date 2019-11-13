#!/bin/sh

# set various options
#export SROGUEOPTS=cutcorners,difficulty=normal

# If we have a saved game, resume it, otherwise start a new one.
if [ -e ~/rogue.save ] ; then
    urogue ~/rogue.save
else
    # add game options to the following line, e.g. "urogue -easy"
    urogue $@
fi
