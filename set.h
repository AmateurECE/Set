/*******************************************************************************
 * NAME:	    set.h
 *
 * AUTHOR:	    Ethan D. Twardy
 *
 * DESCRIPTION:	    The header file for the implementation of a 'set' as an
 *		    abstract data type. Contained herein is the typedefs and
 *		    macros for the source in set.c
 *
 * CREATED:	    05/09/17
 *
 * LAST EDITED:	    06/08/17
 ***/

#ifndef __ET_SET_H__
#define __ET_SET_H__

/*******************************************************************************
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

/*******************************************************************************
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

#define set_intersection(Seti, ...) \
  (set_intersection_func(Seti, (Set * []){__VA_ARGS__, NULL}))

/*******************************************************************************
 * API FUNCTION PROTOTYPES
 ***/

extern void set_init(Set *,
		     int (*)(const void *, const void *),
		     void (*)(void *));
extern int set_ismember(const Set *, const void *);
extern int set_insert(Set *, void *);
extern int set_remove(Set *, void **);
extern int set_traverse(Set *, void (*)(void *));
extern void set_destroy(Set *);

/************* SET OPERATIONS ***************/
/********************************************/

/* These functions: */
extern int set_union_func(Set *, Set * []);
extern int set_intersection_func(Set *, Set * []);
/* Should NEVER be called directly. Use the wrapper macros defined above. */

extern int set_difference(Set *, const Set *, const Set *);
extern int set_issubset(const Set *, const Set *);
extern int set_isequal(const Set *, const Set *);

#endif /* __ET_SET_H__ */

/******************************************************************************/
