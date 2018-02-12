/******************************************************************************
 * NAME:	    set.c
 *
 * AUTHOR:	    Ethan D. Twardy
 *
 * DESCRIPTION:	    Source file containing code for implementing a set--a
 *		    fundamental discrete structure used for solving problems in
 *		    discrete mathematics. This code follows the typedefs and
 *		    prototypes contained in set.h. Also included is source code
 *		    for testing the structure. Compile this by 'make debug.'
 *
 * CREATED:	    05/09/2017
 *
 * LAST EDITED:	    02/12/2018
 ***/

/******************************************************************************
 * INCLUDES
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "set.h"

/******************************************************************************
 * API FUNCTIONS
 ***/

/******************************************************************************
 * FUNCTION:	    set_create
 *
 * DESCRIPTION:	    Initializes the set pointer with the parameters given.
 *
 * ARGUMENTS:	    match: (int (*)(const void *, const void *)) -- a pointer
 *			to a user-defined function that compares two keys
 *			and determines if they are equal. Should return 1
 *			for equality and 0 otherwise.
 *		    copy: (void * (*copy)(const void *)) -- pointer to a user-
 *			defined function which copies the user-data passed to
 *			it and returns a pointer to the copy. If this function
 *			returns NULL, the set API assumes an error has occurred
 *			and returns immediately with a proper error code. This
 *			argument can be NULL, but if it is, all of the
 *			set operations which create a new set (set_union, etc)
 *			will exit immediately with an error code.
 *		    destroy: (void (*)(void *)) -- a pointer to a user-defined
 *			function that frees data held in the list.
 *
 * RETURN:	    (set *) -- pointer to the new set, or NULL.
 *
 * NOTES:	    O(1)
 ***/
set * set_create(int (*match)(const void *, const void *),
		 void * (*copy)(const void *),
		 void (*destroy)(void *))
{
  if (match == NULL)
    return NULL;
  set * group = NULL;
  if ((group = malloc(sizeof(set))) == NULL)
    return NULL;

  *group = (set){
    .size = 0,
    .match = match,
    .copy = copy,
    .destroy = destroy,
    .head = NULL,
    .tail = NULL
  };

  return group;
}

/******************************************************************************
 * FUNCTION:	    set_ismember
 *
 * DESCRIPTION:	    Determines if the key provided in 'data' is already a
 *		    member of the set.
 *
 * ARGUMENTS:	    group: (const set *) -- the set to be operated on.
 *		    data: (const void *) -- data to check.
 *
 * RETURN:	    int -- 1 if the member is in the set, 0 if it is not, -1 if
 *		    an error has occurred.
 *
 * NOTES:	    O(n)
 ***/
int set_ismember(const set * group, const void * data)
{
  if (group == NULL || data == NULL || set_isempty(group))
    return 0;

  member * current = group->head;
  while ((group->match(current->data, data) != 1) && set_next(current))
    ;

  if (current != NULL)
    return group->match(current->data, data);
  else
    return 0;
}

/******************************************************************************
 * FUNCTION:	    set_insert
 *
 * DESCRIPTION:	    Inserts the 'data' into the set if it does not already
 *		    exist in the set.
 *
 * ARGUMENTS:	    group: (set *) -- the set to be operated on.
 *		    data: (const void *) -- data to insert.
 *
 * RETURN:	    int -- 0 if successful, 1 if the data is already contained
 *		    in the set, -1 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_insert(set * group, void * data)
{
  if (group == NULL || data == NULL)
    return -1;

  if (set_ismember(group, data))
    return 1;

  member * new = (member *)malloc(sizeof(member));
  new->data = data;

  if (set_isempty(group)) {

    group->head = new;
    group->tail = new;
    new->next = NULL;

  } else {

    group->tail->next = new;
    group->tail = new;
    new->next = NULL;

  }  

  group->size++;
  return 0;
}

/******************************************************************************
 * FUNCTION:	    set_remove
 *
 * DESCRIPTION:	    Removes the member specified in 'data' from the set. If
 *		    *data == NULL, removes the first member from the set.
 *
 * ARGUMENTS:	    group: (set *) -- the set to be operated on.
 *		    data: (void **) -- data to remove.
 *
 * RETURN:	    int -- 0 if successful, -1 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_remove(set * group, const void ** data)
{
  if (group == NULL || data == NULL || *data == NULL)
    return -1;

  if (set_ismember(group, *data) != 1 && *data != NULL)
    return -1;

  member * old;
  if (*data == NULL) {
    /* Remove the first element in the set. */
    old = group->head;
    group->head = group->head->next;

    if (group->head == NULL)
      group->tail = NULL;
  } else {

    member * current = group->head;
    if (group->match(current->data, *data) != 1) {
      while (group->match(current->next->data, *data) != 1) {
	current = current->next;
      }

      old = current->next;
      if (current->next == group->tail)
	current->next = NULL;
      else
	current->next = current->next->next;

    } else {

      old = current;
      group->head = group->head->next;
      if (group->head == NULL)
	group->tail = NULL;

    }
  }

  if (group->destroy != NULL)
    group->destroy(old->data);
  free(old);
  group->size--;
  return 0;
}

/******************************************************************************
 * FUNCTION:	    set_traverse
 *
 * DESCRIPTION:	    Traverses the set and calls func() on each member in the 
 *		    set. Since the type member is unkown to the user, the
 *		    function takes a pointer to void.
 *
 * ARGUMENTS:	    group: (set *) -- the set to be operated on.
 *		    func: (void (*)(void *) -- the function to be called on
 *			  each member of the list.
 *
 * RETURN:	    int -- 0 if successful, -1 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_traverse(set * group, void (*func)(void *))
{
  member * current;

  if (group == NULL || set_isempty(group) || func == NULL)
    return -1;

  for (current = group->head; current != NULL; set_next(current))
    func(current->data);

  return 0;
}

/******************************************************************************
 * FUNCTION:	    set_destroy
 *
 * DESCRIPTION:	    Removes all data from the set and sets all bytes of memory
 *		    to 0. If destroy is set to NULL, does not attempt to free
 *		    the memory associated with the data in the set.
 *
 * ARGUMENTS:	    group: (set **) -- the set to be operated on.
 *
 * RETURN:	    void.
 *
 * NOTES:	    O(n)
 ***/
void set_destroy(set ** group)
{
  if (group == NULL || *group == NULL)
    return;

  void * data;
  member * old;
  while (set_size(*group) > 0) {
    data = (*group)->head->data;
    old = (*group)->head;
    (*group)->head = (*group)->head->next;
    
    (*group)->size--;
    free(old);
    
    if ((*group)->destroy != NULL) {
      (*group)->destroy(data);
    }
  }

  free(*group);
  *group = NULL;
}

/******************************************************************************
 * FUNCTION:	    set_union_func
 *
 * DESCRIPTION:	    Performs the union set operation and places the result in
 *		    setu. This function uses the set->copy parameter, to
 *		    prevent memory dependencies in the new set. If this param
 *		    is set to NULL or returns NULL, this function exits
 *		    immediately.
 *
 * ARGUMENTS:	    setu: (set **) -- will contain a pointer to the union of
 *			all sets at the end of the call.
 *		    sets: (set * []) -- An array of sets to operate on. If the
 *			set_union() macro was used (as it should be), the final
 *			set in the array will be NULL.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn), where m is the number of sets unioned. Should always
 *		    be called by wrapper macro.
 ***/
int set_union_func(set ** setu,  set * sets[])
{
  if (sets[0] == NULL || setu == NULL)
    return -1;
  if ((*setu = set_create(sets[0]->match,
			  sets[0]->copy,
			  sets[0]->destroy)) == NULL)
    return -1;

  int i = 0;
  for (set * set = sets[i]; set != NULL; set = sets[i++]) {
    if (set->copy == NULL)
      goto error_exception;
    int ret = 0;
    for (member * current = set->head; current != NULL; set_next(current)) {
      void * new = NULL;
      if ((new = (*setu)->copy(current->data)) == NULL)
	goto error_exception;
      if ((ret = set_insert(*setu, new))) {
	if ((*setu)->destroy != NULL)
	  (*setu)->destroy(new);
	if (ret < 0)
	  goto error_exception;
      }
    }
  }

  return 0;

 error_exception: {
    set_destroy(setu);
    return -1;
  }
}

/******************************************************************************
 * FUNCTION:	    set_intersection_func
 *
 * DESCRIPTION:	    Performs the intersection set operation and places the
 *		    result in seti.
 *
 * ARGUMENTS:	    seti: (set **) -- will contain a pointer to the
 *			intersection of all sets at the end of the call. May be
 *			NULL, or the empty set. All other values return error.
 *		    sets: (set * []) -- An array of sets to operate on. If the
 *			set_union() macro was used (as it should be), the final
 *			set in the array will be NULL.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn), where m is the number of sets passed.
 ***/
int set_intersection_func(set ** seti, set * sets[])
{
  if (sets[0] == NULL || seti == NULL || sets[0]->copy == NULL)
    return -1;
  int i = 0;
  for (set * s = sets[i]; s != NULL; s = sets[++i])
    if (s->copy == NULL)
      return -1;
  if ((*seti = set_create(sets[0]->match,
			  sets[0]->copy,
			  sets[0]->destroy)) == NULL)
    return -1;

  for (member * current = (*sets)->head; current != NULL; set_next(current)) {
    int nonmember = 0, j = 1;
    while (sets[j] != NULL && !nonmember)
      nonmember = !set_ismember((const set *)sets[j++], current->data);

    if (nonmember)
      continue;
    set_insert(*seti, (*seti)->copy(current->data));
  }

  return 0;
}

/******************************************************************************
 * FUNCTION:	    set_difference
 *
 * DESCRIPTION:	    Performs the set difference operation and places the
 *		    result in setd.
 *
 * ARGUMENTS:	    setd: (set **) -- will contain a pointer to the difference
 *			of all sets at the end of the call.
 *		    set1: (const set *) -- the minuend of the subtraction.
 *		    set2: (const set *) -- the difference of the subtraction.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn)
 ***/
int set_difference(set ** setd, const set * set1, const set * set2)
{
  if (setd == NULL || set1 == NULL || set2 == NULL
      || set1->copy == NULL || set2->copy == NULL)
    return -1;
  if ((*setd = set_create(set1->match,
			  set1->copy,
			  set1->destroy)) == NULL)
    return -1;

  member * current;
  for (current = set1->head; current != NULL; set_next(current)) {
    
    if (!set_ismember(set2, current->data))
      if (set_insert(*setd, (*setd)->copy(current->data)) != 0)
	goto error_exception;

  }

  return 0;

 error_exception: {
    set_destroy(setd);
    return -1;
  }
}

/******************************************************************************
 * FUNCTION:	    set_issubset
 *
 * DESCRIPTION:	    Determines if set1 is a subset of set2.
 *
 * ARGUMENTS:	    set1: (const set *) -- the set in question.
 *		    set2: (const set *) -- the reference set.
 *
 * RETURN:	    int -- 1 if the set is a subset, 0 otherwise.
 *
 * NOTES:	    O(mn)
 ***/
int set_issubset(const set * set1, const set * set2)
{
  if (set1 == NULL || set2 == NULL)
    return 0;

  if (set_isempty(set1) && !set_isempty(set2))
    return 1;

  member * current;
  for (current = set1->head; current != NULL; set_next(current)) {
    
    if (!set_ismember(set2, current->data))
      return 0;

  }

  return 1;
}

/******************************************************************************
 * FUNCTION:	    set_isequal_func
 *
 * DESCRIPTION:	    Determines if set1 is equal to set2.
 *
 * ARGUMENTS:	    sets: (set * []) - the sets to check equality of.
 *
 * RETURN:	    int -- 1 if the sets are equal, 0 otherwise.
 *
 * NOTES:	    O(mn)
 ***/
int set_isequal_func(set * sets[])
{
  if (sets[0] == NULL)
    return 0;
  int i = 0;
  for (set * s = sets[i]; s != NULL; s = sets[++i])
    if (set_size(sets[i]) != set_size(sets[0]))
      return 0;
  if (i == 1)
    return 0;

  member * current;
  for (current = sets[0]->head; current != NULL; set_next(current)) {
    i = 0;
    for (set * s = sets[i]; s != NULL; s = sets[++i])
      if (!set_ismember(s, current->data))
	return 0;
  }
  
  return 1;
}

/******************************************************************************
 * FUNCTION:	    set_copy
 *
 * DESCRIPTION:	    Produces a deep copy of the set `s'.
 *
 * ARGUMENTS:	    s: (set *) -- the set to create a copy of.
 *
 * RETURN:	    set * -- a deep copy of the set `s', or NULL if an error
 *		    has occurred.
 *
 * NOTES:	    O(n)
 ***/
set * set_copy(const set * s)
{
  if (s == NULL)
    return NULL;
  if (set_size(s) == 0)
    return set_create(s->match, s->copy, s->destroy);

  set * _new = NULL;
  if ((_new = set_create(s->match, s->copy, s->destroy)) == NULL)
    return NULL;

  for (member * current = s->head; current != NULL;
       current = current->next) {
    if (set_insert(_new, _new->copy(current->data)) < 0) {
      set_destroy(&_new);
      return NULL;
    }
  }

  return _new;
}

/*****************************************************************************/
