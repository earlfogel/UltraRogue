# UltraRogue 1.0.8

### Table of Contents
* [Introduction](#introduction)
* [What's New?](#whats-new)
* [Getting Started](#getting-started)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
* [Authors](#authors)
* [License](#license)

## Introduction

UltraRogue is an old ascii-graphics game developed by Herb Chong in the
1980s and 1990s.  It's one of many roguelike games inspired by Dungeons and
Dragons and by the original Unix game Rogue.

I fell in love with urogue in the mid-1980s.  Later versions added
more features and monsters, but the game also became more cumbersome
to play.

This feels like the original urogue, but with more monsters,
working save/restore code, some new and backported features and many bug fixes.
I also streamlined gameplay to reduce typing and to lessen the strain on
aging wrists.

If you have fond memories of the original urogue or other early roguelikes,
then you may like this one too.

![Screenshot](hobgoblin.png?raw=true)

## What's New?

A variety of bug fixes and tweaks, most of which are detailed in the
CHANGELOG.  Also, ...

### Difficulty Levels

There are two command line options to adjust the level of difficulty of the
game, -easy and -hard.

In easy games, your pack can hold more items and you start out with
more stuff.  There are fewer traps and fewer cursed items, and you are
less likely to be summoned to a throne room.  Also, armor is more
effective.

In hard games, there are fewer blessed items, artifacts are more dangerous,
and armor is less effective.  You are more vulnerable to magical
attacks and your ability to do magic is reduced.  You can't start a hard
game with plate, mithril or crystalline armor, but you may find some later.

There is also a Very Hard level.  Getting into this difficulty level is
left as an exercise for the player.

### New Options

If you turn on Autosave, urogue will save the game periodically
and lets you roll back to the most recent save point when you die.
I.e, no permadeath.

If you turn off Autopickup (i.e. noautopickup) then your character
won't pick things up automatically when you step on them.

If you turn on Cutcorners, you can move and fight around corners,
as can the more dexterous monsters.

You can set these (and most other) options using the 'o' command in urogue,
or via the SROGUEOPTS environment variable.

### Monsters

To keep things interesting, urogue chooses a random selection of monsters
each time you play.  By default, that includes the classic monsters from
urogue 1.0.2 plus a random assortment of others.  If you want, you can use
command line options to change this:

    * -mc: use only the classic monsters from urogue 1.0.2
    * -mr: use a random selection of monsters
    * -ma: use all 400+ monsters

### Fighting

Use the 'f' command to fight a single monster.  If there is more than one
monster in reach, urogue asks which you want to fight.

Use the 'F' command to fight a group of monsters. I.e., when the monster
you are fighting moves or dies, urogue picks another and keeps going.  Fights end
when there are no more monsters in reach, or when you are too weak, sick,
hungry or injured to continue.

There are fewer interruptions when fighting, and fewer messages to
distract you.  If you are much stronger than the monsters around
you, this means you can press 'F' and then press space until they
all disappear.

With the 'F' command, urogue tries to avoid friendly
monsters and monsters that divide when hit.

\<CTRL\>-F (serious fight) is like 'F' but fights last longer and
include friendly and dividing monsters.

### Running Around

Pressing \<SHIFT\> and a direction key runs until you reach something
interesting.  There is an option (nodoorstop) that lets you run into things
(as in classic rogue).

Pressing \<CTRL\> and a direction key does a controlled run -- where you
stop and search after every step and don't pick things up when you step on
them.  This is particularly useful in treasure chambers, but it does mean
that any monsters that are chasing you quickly catch up.

## Getting Started

As in other early roguelikes, in urogue you use keyboard
commands to wander through a dungeon fighting monsters and collecting
treasure.  The dungeon, monsters and treasure are all displayed on screen
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

These keys move one space at a time. To speed things up, press \<SHIFT\> and
a movement key to run until you reach something interesting.  Press \<CTRL\>
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
    o	examine/set options
    p	pray
    q	quaff a potion
    r	read a scroll
    s	search for trap/secret door
    z	zap a magic wand
    S	save game
    Q	quit

To restore a saved game from the default location, just run urogue again.
To restore a different saved game, use "urogue \<filename\>".  If you don't
want to restore a saved game, run "urogue -new" (or delete the save file).

Older urogue versions (which may no longer be playable) are available
from the roguelike archive at: https://britzl.github.io/roguearchive/

## Prerequisites

A C compiler, a curses library, and make.

## Installation

This software builds on linux and windows.  Probably other unix's too,
but I haven't tried any recently.

To build urogue, change to the rogue sub-directory and type make:

    cd rogue
    make

Then run 'urogue' in an 80x24 or larger terminal window.

See the INSTALL file if you'd like to build for Windows.

## Authors

UltraRogue was originally written by Herb Chong, with many additional
contributors.  The Roguelike Restoration Project released urogue 1.0.7
in 2005.

This is version 1.0.8 of urogue, maintained by Earl Fogel.

## License

See the LICENSE.TXT file for details.



















