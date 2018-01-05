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
 * LAST EDITED:	    01/04/2018
 ***/

#ifndef __ET_SET_H__
#define __ET_SET_H__

/******************************************************************************
 * TYPE DEFINITIONS
 ***/

typedef struct _Member_ {

  void * data;
  struct _Member_ * next;

} Member;

typedef struct _Set_ {

  int size;

  int (*match)(const void *, const void *);
  void (*destroy)(void *);

  Member * head;
  Member * tail;

} Set;

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
  (set_union_func(Setu, (Set * []){__VA_ARGS__, NULL}))

#define set_intersection(Seti, ...)				\
  (set_intersection_func(Seti, (Set * []){__VA_ARGS__, NULL}))

/******************************************************************************
 * API FUNCTION PROTOTYPES
 ***/

extern void set_init(Set * set,
		     int (*match)(const void *, const void *),
		     void (*destroy)(void *));
extern int set_ismember(const Set * set, const void * data);
extern int set_insert(Set * set, void * data);
extern int set_remove(Set * set, void ** data);
extern int set_traverse(Set * set, void (*func)(void *));
extern void set_destroy(Set * set);

/* These functions: */
extern int set_union_func(Set *, Set * []);
extern int set_intersection_func(Set *, Set * []);
/* Should NEVER be called directly. Use the wrapper macros defined above. */

extern int set_difference(Set * dest,
			  const Set * source1,
			  const Set * source2);
extern int set_issubset(const Set * subset, const Set * masterset);
extern int set_isequal(const Set * set1, const Set * set2);

#endif /* __ET_SET_H__ */

/*****************************************************************************/
