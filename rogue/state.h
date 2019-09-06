/*
 * Definitions backported from Urogue 1.0.7
 * for save and restore (state.c)
 *
 */
#include <string.h>

#if 0
#define MAXDAEMONS 60
#define MAXMAGICTYPES   4
#define MAXMAGICITEMS   50
#define C_NOTSET   11
#define INV_CLEAR   2
#endif

extern void ur_write_thing(FILE *savef, struct thing *t);
extern struct thing *ur_read_thing(FILE *savef);
extern void ur_write_object_stack(FILE *savef, struct linked_list *l);
extern void ur_write_bag(int isbag, FILE *savef, struct linked_list *l);
extern struct linked_list *ur_read_bag(int isbag, FILE *savef);
extern struct linked_list *ur_read_object_stack(FILE *savef);
extern int restore_file(FILE *savef);
extern void ur_read_daemons(FILE *savef);
extern void ur_write_daemons(FILE *savef);
extern void ur_write_artifact(FILE *savef, struct artifact *a);
extern struct artifact *ur_read_artifact(FILE *savef);
extern void do_after_effects(void);
extern void * ur_alloc(unsigned int size);
extern void ur_free(void *buf_p);
extern char pack_letters[];
extern char *pack_index;
extern char bag_letters[];
extern char *bag_index;
extern int demoncnt;
