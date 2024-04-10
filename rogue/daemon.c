/*
    daemon.c - functions for dealing with things that happen in the future
*/

/*
        Needs to be rewritten again to work on a per creature basis.
        Either each monster will have a list of effect, or each
        fuse will take a creature pointer and multiple entries
        for each fuse will be allowed. I tend to want to attach
        the effects to the creature.
*/

#include <stdarg.h>
#include "curses.h"
#include "rogue.h"

int demoncnt;

struct daemon daemons[DAEMON_MAX] =
{
    { DAEMON_NULL,       NULL         },
    { DAEMON_DOCTOR,     doctor       },
    { DAEMON_ROLLWAND,   rollwand     },
    { DAEMON_STOMACH,    stomach      },
    { DAEMON_RUNNERS,    runners      }
};

struct fuse fuses[FUSE_MAX] =
{
    { FUSE_NULL,         NULL         },
    { FUSE_SWANDER,      swander      },
    { FUSE_UNCONFUSE,    unconfuse    },
    { FUSE_UNSCENT,      unscent      },
    { FUSE_SCENT,        scent        },
    { FUSE_UNHEAR,       unhear       },
    { FUSE_HEAR,         hear         },
    { FUSE_UNSEE,        unsee        },
    { FUSE_UNSTINK,      unstink      },
    { FUSE_UNCLRHEAD,    unclrhead    },
    { FUSE_UNPHASE,      unphase      },
    { FUSE_SIGHT,        sight        },
    { FUSE_RES_STRENGTH, res_strength },
    { FUSE_NOHASTE,      nohaste      },
    { FUSE_NOSLOW,       noslow       },
    { FUSE_SUFFOCATE,    suffocate    },
    { FUSE_CURE_DISEASE, cure_disease },
    { FUSE_UNITCH,       un_itch      },
    { FUSE_APPEAR,       appear       },
    { FUSE_UNELECTRIFY,  unelectrify  },
    { FUSE_UNBHERO,      unbhero      },
    { FUSE_UNSHERO,      unshero      },
    { FUSE_UNXRAY,       NULL         },
    { FUSE_UNDISGUISE,   undisguise   },
    { FUSE_SHERO,        shero        },
    { FUSE_WGHTCHK,      wghtchk      },
    { FUSE_CURE_INFEST,  cure_infest  },
    { FUSE_UNFLY,        unfly        },
};

/*
    d_slot()
        Find an empty slot in the daemon/fuse list
*/

struct delayed_action *
d_slot(void)
{
    int i;
    struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
        if (dev->d_type == EMPTY)
            return(dev);

    return(NULL);
}


/*
     find_slot()
         Find a particular slot in the table
*/

struct delayed_action *
find_slot(int type, int id)
{
    int i;
    struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
        if ( (dev->d_type == type) && (id == dev->d_id) )
            return(dev);

    return(NULL);
}


/*
    daemon()
        Start a daemon, takes a function.
*/

void
start_daemon(int id, void *arg, int whendo)
{
    struct delayed_action *dev;

    dev = d_slot();

    if (dev != NULL)
    {
        dev->d_type = DAEMON;
        dev->d_when = whendo;
        dev->d_id = id;
        dev->d_arg = arg;
        dev->d_time = 1;
        demoncnt += 1;  /* update count */
    }
}


/*
    kill_daemon()
        Remove a daemon from the list
*/

void
kill_daemon(int id)
{
    struct delayed_action *dev;

    if ((dev = find_slot(DAEMON, id)) == NULL)
        return;

    /* Take it out of the list */

    dev->d_type = EMPTY;
    demoncnt -= 1;      /* update count */

    return;
}


/*
    do_daemons()
        Run all the daemons that are active with the current flag,
        passing the argument to the function.
*/

void
do_daemons(int now)
{
    struct delayed_action *dev;

    /* Loop through the devil list */

    for (dev = d_list; dev < &d_list[MAXDAEMONS]; dev++)
        /* Executing each one, giving it the proper arguments */
        if ( (dev->d_when == now) && (dev->d_type == DAEMON))
        {
            if ((dev->d_id < 1) || (dev->d_id >= DAEMON_MAX))
                printf("Bad daemon id %d\n", dev->d_id);
            else if (daemons[dev->d_id].func == NULL)
                printf("No action for daemon %d!!!\n", dev->d_id);
            else
            {
                daemon_arg arg;
				
                arg.varg = dev->d_arg;
                daemons[dev->d_id].func(&arg);
            }
        }

}


/*
    fuse()
        Start a fuse to go off in a certain number of turns
*/

void
light_fuse(int id, void *arg, int time, int whendo)
{
    struct delayed_action   *wire;

    wire = d_slot();

    if (wire != NULL)
    {
        wire->d_type = FUSE;
        wire->d_when = whendo;
        wire->d_id   = id;
        wire->d_arg  = arg;
        wire->d_time = time;
        demoncnt += 1;  /* update count */
    }
}


/*
    lengthen()
        Increase the time until a fuse goes off
*/

void
lengthen_fuse(int id, int xtime)
{
    struct delayed_action *wire;

    if ((wire = find_slot(FUSE,id)) == NULL)
        return;

#ifdef EARL
if (id == FUSE_UNCONFUSE && wire->d_time > 100)
return;
#endif

    wire->d_time += xtime;

    return;
}


/*
    extinguish()
        Put out a fuse
*/

void
extinguish_fuse(int id)
{
    struct delayed_action   *wire;

    if ((wire = find_slot(FUSE,id)) == NULL)
        return;

    wire->d_type = EMPTY;
    demoncnt -= 1;

    return;
}


/*
    do_fuses()
        Decrement counters and start needed fuses
*/

void
do_fuses(int now)
{
    struct delayed_action   *wire;

    /* Step though the list */

    for (wire = d_list; wire < &d_list[MAXDAEMONS]; wire++)
    {
        /*
         * Decrementing counters and starting things we want.  We
         * also need to remove the fuse from the list once it has
         * gone off.
         */

        if( (wire->d_type == FUSE) && (wire->d_when == now) )
        {
            if (--wire->d_time <= 0)
            {
                fuse_arg arg;

                arg.varg = wire->d_arg;
                wire->d_type = EMPTY;
                fuses[wire->d_id].func(&arg);
                demoncnt -= 1;
            }
        }

    }

    return;
}


/*
    activity()
        Show wizard number of demaons and memory blocks used
*/

void
activity(void)
{
    msg("Daemons = %d : Memory Items = %d ", demoncnt, total);
    return;
}

/*
    waste_time()
        Do nothing but let other things happen
*/

void
waste_time(void)
{
    if (inwhgt)     /* if from wghtchk then done */
        return;

    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
