#!/bin/sh

if [ -e ~/rogue.save ] ; then
    urogue ~/rogue.save
else
    # add game options to the following line, e.g. "urogue -easy"
    urogue
fi
