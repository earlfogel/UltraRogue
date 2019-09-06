# UltraRogue Remix

UltraRogue is an old ascii-graphics game developed by Herb Chong in the
1980s and 1990s.  It's one of many roguelike games inspired by Dungeons and
Dragons and by the original game Rogue.

For this version of urogue, I pulled code from several old versions,
so it's something of a remix.  I wanted to keep the feel of the classic
urogue, but to streamline the gameplay to reduce the strain on aging
wrists.

## Prerequisites

A C compiler, the ncurses library, and make.

## Installation

This software builds on linux and windows.  Probably other unix's too,
but I haven't tried.

To build urogue, change to the rogue directory and type make:

    cd rogue
    make

Then run 'urogue' in an 80x24 or larger terminal window.

## Getting Started

UltraRogue is an old ascii-graphics game developed by Herb Chong in the
1980s and 1990s.  It's one of many roguelike games inspired by Dungeons and
Dragons and by the original 1980 unix game Rogue.  A few others from
this time period were Advanced Rogue, Hack and Larn.

As in the other roguelike games of this time period, you use keyboard
commands to wander through a dungeon fighting monsters and collecting
treasure.  The dungeon, monsters, and treasure are all displayed on screen
as text characters.  For example, you are an '@' sign, and monsters are
various letters of the alphabet.  The dungeon itself is drawn with dots,
dashes, vertical bars and so on.

There are 8 different magical artifacts in this version of rogue.
The first is on level 25 and is relatively easy to pick up.  The last is
on level 100 and is very difficult to get.  Carrying any of these
treasures allows the rogue to go up the stairs.  To win the game, 
work your way down through the dungeon collecting artifacts, and then climb
back out again.

You can use the arrow keys to move around in this version of urogue, but
it's generally more convenient to use the keyboard movement keys:

    h	left                            
    j	down                            
    k	up                              
    l	right                           
    y	up & left                       
    u	up & right                      
    b	down & left                     
    n	down & right

These keys move one space at a time. To speed things up, press <SHIFT> and
a movement key to run until you reach something interesting.  Press <CTRL>
and a movement key to run, searching at each step.

Here are a few other commands to get you started:

    ?	prints help
    /	identify object                 
    >	go down a staircase
    .	rest
    ,	pick up an object
    c	cast a spell
    d	drop an object
    e	eat food
    i	inventory
    m	move without picking up
    p	pray
    q	quaff a potion
    r	read a scroll
    s	search for trap/secret door
    z	zap a magic wand
    S	save game
    Q	quit

To restore a saved game, use "urogue -r" for the default save file,
or "urogue <filename>" for any other.

Older UltraRogue versions (which may no longer be playable) are available
from the roguelike archive at: https://britzl.github.io/roguearchive/

## Authors

UltraRogue was originally written by Herb Chong, with many additional
contributors.  The Roguelike Restoration Project released UltraRogue 1.0.7
in 2005.

This version of UltraRogue, forked in 2018, is maintained by Earl Fogel.

## License

See the LICENSE.TXT file for details.

