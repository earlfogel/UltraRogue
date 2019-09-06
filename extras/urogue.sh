#!/bin/sh

if [ -e ~/rogue.save ] ; then
    urogue ~/rogue.save
else
    urogue
fi
