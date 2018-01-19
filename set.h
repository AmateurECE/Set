/******************************************************************************
 * NAME:	    set.h
 *
 * AUTHOR:	    Ethan D. Twardy
 *
 * DESCRIPTION:	    The header file for the implementation of a 'set' as an
 *		    abstract data type. Contained herein is the typedefs and
 *		    macros for the source in set.c
 *
 * CREATED:	    05/09/2017
 *
 * LAST EDITED:	    01/18/2018
 ***/

#ifndef __ET_SET_H__
#define __ET_SET_H__

/******************************************************************************
 * TYPE DEFINITIONS
 ***/

typedef struct _member_ {

  void * data;
  struct _member_ * next;

} member;

/* TODO: Rename the set structs
 */

typedef struct {

  int size;

  int (*match)(const void *, const void *);
  void (*destroy)(void *);

  member * head;
  member * tail;

} set;

/******************************************************************************
 * MACRO DEFINITIONS
 ***/

#define set_size(set) ((set)->size)
#define set_isempty(set) (set_size(set) == 0 ? 1 : 0)
/* Mostly for use in for and while loops: */
#define set_next(member) (member = (member)->next)

/* Wrapper macros for set_union and set_intersection
 * These macros safely terminate the list, and should ALWAYS be called
 * instead of set_union_func and set_intersection_func, respectively.
 */
#define set_union(Setu, ...)				\
  (set_union_func(Setu, (set * []){__VA_ARGS__, NULL}))

#define set_intersection(Seti, ...)				\
  (set_intersection_func(Seti, (set * []){__VA_ARGS__, NULL}))

/******************************************************************************
 * API FUNCTION PROTOTYPES
 ***/

extern set * set_create(int (*match)(const void *, const void *),
		       void (*destroy)(void *));
extern int set_ismember(const set * set, const void * data);
extern int set_insert(set * set, void * data);
extern int set_remove(set * set, void ** data);
extern int set_traverse(set * set, void (*func)(void *));
extern void set_destroy(set ** set);

/* These functions: */
extern int set_union_func(set **, set * []);
extern int set_intersection_func(set **, set * []);
/* Should NEVER be called directly. Use the wrapper macros defined above. */

extern int set_difference(set ** dest,
			  const set * source1,
			  const set * source2);
extern int set_issubset(const set * subset, const set * masterset);
extern int set_isequal(const set * set1, const set * set2);

#endif /* __ET_SET_H__ */

/*****************************************************************************/
