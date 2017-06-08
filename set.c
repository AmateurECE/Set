/*******************************************************************************
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
 * CREATED:	    05/09/17
 *
 * LAST EDITED:	    06/08/17
 ***/

/* ======= EXAMPLE OUTPUT =======
 *
 * ==== Inserting ====
 * int 9 @ 0x7fdea2500030
 * int 5 @ 0x7fdea2500050
 * int 4 @ 0x7fdea2500070
 * int 7 @ 0x7fdea2500090
 * int 3 @ 0x7fdea25000b0
 * int 0 @ 0x7fdea25000d0
 * int 7 @ 0x7fdea25000f0: IS A MEMBER
 * int 1 @ 0x7fdea2500100
 * int 8 @ 0x7fdea2500120
 * int 2 @ 0x7fdea2500140
 * int 6 @ 0x7fdea2500160
 * ==== Removing =====
 * int 9 @ 0x7fdea2500030
 * int 5 @ 0x7fdea2500050
 * int 4 @ 0x7fdea2500070
 * int 7 @ 0x7fdea2500090
 * int 3 @ 0x7fdea25000b0
 * int 0 @ 0x7fdea25000d0
 * int 1 @ 0x7fdea2500100
 * int 8 @ 0x7fdea2500120
 * int 2 @ 0x7fdea2500140
 * int 6 @ 0x7fdea2500160
 */

/*******************************************************************************
 * INCLUDES
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef CONFIG_DEBUG_SET
#include <time.h>
#include <assert.h>
#endif /* CONFIG_DEBUG_SET */

#include "set.h"

/*******************************************************************************
 * LOCAL PROTOTYPES
 ***/

#ifdef CONFIG_DEBUG_SET
static inline void error_exit(char *);
int match(const void *, const void *);
void printset(void *);

/* ****** TESTS ****** */
static int test_remove(Set *);
static int test_insert(Set *);
static int test_isequal(Set *, Set *);
static int test_union(Set *, Set *, Set *);
static int test_intersection(Set *, Set *, Set *);
static int test_subtraction(Set *, Set *, Set *);
#endif /* CONFIG_DEBUG_SET */

/*******************************************************************************
 * API FUNCTIONS
 ***/

/*******************************************************************************
 * FUNCTION:	    set_init
 *
 * DESCRIPTION:	    Initializes the set pointer with the parameters given.
 *
 * ARGUMENTS:	    set: (Set *) -- the set to be initialized.
 *		    match: (int (*)(const void *, const void *)) -- a pointer to
 *			   a user-defined function that compares two keys and
 *			   determines if they are equal. Should return 1 for
 *			   equality and 0 otherwise.
 *		    destroy: (void (*)(void *)) -- a pointer to a user-defined
 *			     function that frees data held in the list.
 *
 * RETURN:	    void.
 *
 * NOTES:	    O(1)
 ***/
void set_init(Set * set,
	      int (*match)(const void *, const void *),
	      void (*destroy)(void *))
{
  set->size = 0;
  set->match = match;
  set->destroy = destroy;
  set->head = NULL;
  set->tail = NULL;
}

/*******************************************************************************
 * FUNCTION:	    set_ismember
 *
 * DESCRIPTION:	    Determines if the key provided in 'data' is already a member
 *		    of the set.
 *
 * ARGUMENTS:	    set: (const Set *) -- the set to be operated on.
 *		    data: (const void *) -- data to check.
 *
 * RETURN:	    int -- 1 if the member is in the set, 0 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_ismember(const Set * set, const void * data)
{
  if (data == NULL)
    return -1;

  if (set_isempty(set))
    return 0;

  Member * current = set->head;

  while ((set->match(current->data, data) != 1) && set_next(current))
    ;

  if (current != NULL)
    return set->match(current->data, data);
  else
    return 0;
}

/*******************************************************************************
 * FUNCTION:	    set_insert
 *
 * DESCRIPTION:	    Inserts the 'data' into the set if it does not already exist
 *		    in the set.
 *
 * ARGUMENTS:	    set: (Set *) -- the set to be operated on.
 *		    data: (const void *) -- data to insert.
 *
 * RETURN:	    int -- 0 if successful, 1 if the data is already contained
 *		    in the set, -1 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_insert(Set * set, void * data)
{
  if (data == NULL)
    return -1;

  if (set_ismember(set, data))
    return 1;

  Member * new = (Member *)malloc(sizeof(Member));
  new->data = data;

  if (set_isempty(set)) {

    set->head = new;
    set->tail = new;
    new->next = NULL;

  } else {

    set->tail->next = new;
    set->tail = new;
    new->next = NULL;

  }  

  set->size++;
  return 0;
}

/*******************************************************************************
 * FUNCTION:	    set_rem
 *
 * DESCRIPTION:	    Removes the member specified in 'data' from the set. If
 *		    *data == NULL, removes the first member from the set.
 *
 * ARGUMENTS:	    set: (Set *) -- the set to be operated on.
 *		    data: (void **) -- data to remove.
 *
 * RETURN:	    int -- 0 if successful, -1 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_remove(Set * set, void ** data)
{
  if (set_ismember(set, *data) != 1 && *data != NULL)
    return -1;

  Member * old;

  if (*data == NULL) {
    /* Remove the first element in the set. */
    old = set->head;
    set->head = set->head->next;

    if (set->head == NULL)
      set->tail = NULL;

  } else {

    Member * current = set->head;

    if (set->match(current->data, *data) != 1) {

      while (set->match(current->next->data, *data) != 1) {
	current = current->next;
      }

      old = current->next;

      if (current->next == set->tail)
	current->next = NULL;
      else
	current->next = current->next->next;

    } else {

      old = current;
      set->head = set->head->next;
      
      if (set->head == NULL)
	set->tail = NULL;

    }
  }

  *data = old->data;
  free(old);

  set->size--;
  return 0;
}

/*******************************************************************************
 * FUNCTION:	    set_traverse
 *
 * DESCRIPTION:	    Traverses the set and calls func() on each member in the 
 *		    set. Since the type Member is unkown to the user, the
 *		    function takes a pointer to void.
 *
 * ARGUMENTS:	    set: (Set *) -- the set to be operated on.
 *		    func: (void (*)(void *) -- the function to be called on
 *			  each member of the list.
 *
 * RETURN:	    int -- 0 if successful, -1 otherwise.
 *
 * NOTES:	    O(n)
 ***/
int set_traverse(Set * set, void (*func)(void *))
{
  Member * current;

  if (set == NULL || set_isempty(set))
    return -1;

  for (current = set->head; current != NULL; set_next(current))
    func(current->data);

  return 0;
}

/*******************************************************************************
 * FUNCTION:	    set_dest
 *
 * DESCRIPTION:	    Removes all data from the set and sets all bytes of memory
 *		    to 0. If destroy is set to NULL, does not attempt to free
 *		    the memory associated with the data in the set.
 *
 * ARGUMENTS:	    set: (Set *) -- the set to be operated on.
 *
 * RETURN:	    void.
 *
 * NOTES:	    O(n)
 ***/
void set_destroy(Set * set)
{
  void * data;
  Member * old;

  while (set_size(set) > 0) {
    data = set->head->data;
    old = set->head;
    set->head = set->head->next;
    
    set->size--;
    free(old);
    
    if (set->destroy != NULL) {
      set->destroy(data);
    }
  }

  memset(set, 0, sizeof(Set));
}

/************* SET OPERATIONS ***************/
/********************************************/

/*******************************************************************************
 * FUNCTION:	    set_union
 *
 * DESCRIPTION:	    Performs the union set operation and places the result in
 *		    setu.
 *
 * ARGUMENTS:	    setu: (Set *) -- will contain a pointer to the union of all
 *			  sets at the end of the call.
 *		    numargs: int -- contains the number of arguments passed.
 *		    ...: (Set *) -- all further parameters will be cast to
 *			 pointer to Set and unioned.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn), where m is the number of sets unioned.
 ***/
int set_union(Set * setu, int numargs, ...)
{
  va_list pSet;
  Set * set;
  int first_iteration = 1;

  if (setu == NULL || numargs < 1)
    return -1;

  va_start(pSet, numargs);
  set = va_arg(pSet, Set *);

  set_init(setu, set->match, set->destroy); 

  for (int i = 1; i <= numargs; i++) {
    if (!first_iteration)
      set = va_arg(pSet, Set *);
    else
      first_iteration = 0;

    if (set == NULL)
      goto error_exception;

    Member * current;
    void * data;
    for (current = set->head; current != NULL; set_next(current)) {

      data = current->data;

      if (set_insert(setu, (void *)data) < 0) {
	goto error_exception;
      }
    }
  }

  va_end(pSet);
  return 0;

 error_exception: {
    set_destroy(setu);
    return -1;
  }
}

/*******************************************************************************
 * FUNCTION:	    set_intersection
 *
 * DESCRIPTION:	    Performs the intersection set operation and places the
 *		    result in seti.
 *
 * ARGUMENTS:	    seti: (Set *) -- will contain a pointer to the intersection
 *			  of all sets at the end of the call.
 *		    numargs: int -- contains the number of arguments passed.
 *		    ...: (Set *) -- all further parameters will be cast to
 *			 pointer to Set and intersected.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn), where m is the number of sets passed.
 ***/
int set_intersection(Set * seti, int numargs, ...)
{
  va_list pSet;
  Set * set;
  Set ** sets = calloc(numargs, sizeof(Set *));
  int first_iteration = 1;

  if (seti == NULL || numargs < 1)
    return -1;

  va_start(pSet, numargs);
  set = va_arg(pSet, Set *);

  set_init(seti, set->match, set->destroy);

  for (int i = 0; i < numargs; i++) {
    if (!first_iteration)
      set = va_arg(pSet, Set *);
    else
      first_iteration = 0;

    if (set == NULL)
      goto error_exception;

    sets[i] = set;
  }

  Member * current;
  void * data;
  int nonmember;
  for (current = (*sets)->head; current != NULL; set_next(current)) {

    nonmember = 0;
    data = current->data;
    for (int i = 1; i < numargs; i++) {

      if (!set_ismember(sets[i], data)) {
	nonmember = 1;
	break;
      }

    }

    if (nonmember)
      continue;

    set_insert(seti, data);
  }

  return 0;

 error_exception: {
    set_destroy(seti);
    return -1;
  }
}

/*******************************************************************************
 * FUNCTION:	    set_difference
 *
 * DESCRIPTION:	    Performs the set difference operation and places the
 *		    result in setd.
 *
 * ARGUMENTS:	    setd: (Set *) -- will contain a pointer to the difference
 *			  of all sets at the end of the call.
 *		    set1: (const Set *) -- the minuend of the subtraction.
 *		    set2: (const Set *) -- the difference of the subtraction.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn)
 ***/
int set_difference(Set * setd, const Set * set1, const Set * set2)
{
  if (setd == NULL || set1 == NULL || set2 == NULL)
    return -1;

  set_init(setd, set1->match, set1->destroy);

  Member * current;
  for (current = set1->head; current != NULL; set_next(current)) {
    
    if (!set_ismember(set2, current->data))
      if (set_insert(setd, current->data) != 0)
	goto error_exception;

  }

  return 0;

 error_exception: {
    set_destroy(setd);
    return -1;
  }
}

/*******************************************************************************
 * FUNCTION:	    set_issubset
 *
 * DESCRIPTION:	    Determines if set1 is a subset of set2.
 *
 * ARGUMENTS:	    set1: (const Set *) -- the set in question.
 *		    set2: (const Set *) -- the reference set.
 *
 * RETURN:	    int -- 1 if the set is a subset, 0 otherwise.
 *
 * NOTES:	    O(mn)
 ***/
int set_issubset(const Set * set1, const Set * set2)
{
  if (set1 == NULL || set2 == NULL)
    return 0;

  if (set_isempty(set1) && !set_isempty(set2))
    return 1;

  Member * current;
  for (current = set1->head; current != NULL; set_next(current)) {
    
    if (!set_ismember(set2, current->data))
      return 0;

  }

  return 1;
}

/*******************************************************************************
 * FUNCTION:	    set_isequal
 *
 * DESCRIPTION:	    Determines if set1 is equal to set2.
 *
 * ARGUMENTS:	    set1: (const Set *) -- the set in question.
 *		    set2: (const Set *) -- the reference set.
 *
 * RETURN:	    int -- 1 if the sets are equal, 0 otherwise.
 *
 * NOTES:	    O(mn)
 ***/
int set_isequal(const Set * set1, const Set * set2)
{
  if (set1 == NULL || set2 == NULL || set_size(set1) != set_size(set2))
    return 0;

  Member * current;
  for (current = set1->head; current != NULL; set_next(current)) {
    
    if (!set_ismember(set2, current->data))
      return 0;

  }
  
  return 1;
}

/*******************************************************************************
 * MAIN
 ***/

#ifdef CONFIG_DEBUG_SET
int main(int argc, char * argv[])
{
  /* STANDARD TEST. */
  Set * set;
  int * pNum;

  if ((set = malloc(sizeof(Set))) == NULL)
    error_exit("Could not allocate memory for set!");

  set_init(set, match, free);

  srand((unsigned)time(NULL));

  /* Inserting */
  printf("==== Inserting ====\n");
  while (set_size(set) < 10) {
    pNum = malloc(sizeof(int));
    *pNum = rand() % 10;
    printf("int %d @ %p", *pNum, pNum);
    int ret = set_insert(set, (void *)pNum);
    if (ret < 0)
      printf(": FAILED\n");
    else if (ret > 0)
      printf(": IS A MEMBER\n");
    else
      printf("\n");
  }

  /* Removing */
  printf("==== Removing =====\n");
  while (set_size(set) > 0) {
  pNum = NULL;
    if (set_remove(set, (void **)&pNum) < 0)
      error_exit("Error in set_remove");
    else
      printf("int %d @ %p\n", *pNum, pNum);
    free(pNum);
  }

  set_destroy(set);

  assert(test_remove(set) == 1);
  assert(test_insert(set) == 1);

  Set * set2, * set3;
  if ((set2 = malloc(sizeof(Set))) == NULL
      || (set3 = malloc(sizeof(Set))) == NULL)
    error_exit("There was a problem in MAIN: malloc");

  assert(test_isequal(set, set2));
  assert(test_union(set, set2, set3));
  assert(test_intersection(set, set2, set3));
  assert(test_subtraction(set, set2, set3));

  return 0;
}
#endif /* CONFIG_DEBUG_SET */

/*******************************************************************************
 * LOCAL FUNCTIONS
 ***/

#ifdef CONFIG_DEBUG_SET
static inline void error_exit(char * msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

/* For set_init, and set operations. */
int match(const void * one, const void * two)
{
  int a = *((int *)one);
  int b = *((int *)two);

  if (a == b)
    return 1;
  else
    return 0;
}

/* For set_traverse; Used for debugging */
void printset(void * data)
{
  int * pInt = (int *)data;
  printf("%d ", *pInt);
}

/* ****** TESTS ****** */

static int test_remove(Set * set)
{
  set_init(set, match, free);
  int * pTest = malloc(sizeof(int));
  *pTest = 1;
  int ret = set_remove(set, (void **)&pTest);
  set_destroy(set);
  free(pTest);
  return ret == -1;
}

static int test_insert(Set * set)
{
  set_init(set, match, free);
  int a = 1;
  int * pTest = malloc(sizeof(int));
  *pTest = a;
  if (set_insert(set, (void *)pTest) < 0)
    error_exit("There was a problem in test_insert: set_insert");
  
  pTest = realloc(pTest, sizeof(int));
  *pTest = a;
  int ret = set_insert(set, (void *)pTest);
  set_destroy(set);
  return ret == 1;
}

static int test_isequal(Set * A, Set * B)
{
  /* {1} = {1} */
  set_init(A, match, free);
  set_init(B, match, free);

  int * pA = malloc(sizeof(int));
  int * pB = malloc(sizeof(int));
  *pA = 1, *pB = 1;

  if (set_insert(A, pA) != 0 || set_insert(B, pB) != 0)
    error_exit("There was a problem in test_isequal: set_insert");

  int ret = set_isequal(A, B);
  set_destroy(A);
  set_destroy(B);
  return ret;
}

static int test_union(Set * A, Set * B, Set * U)
{
  /* {0, 1, 2} U {2, 4, 6} = {0, 1, 2, 4, 6} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};
  int arrU[] = {0, 1, 2, 4, 6};
  int * pTest;

  set_init(A, match, NULL);
  set_init(B, match, NULL);
  
  for (int i = 0; i < 3; i++)
    set_insert(A, (void *)&(arrA[i]));

  for (int i = 0; i < 3; i++)
    set_insert(B, (void *)&(arrB[i]));

  if (set_union(U, 2, A, B) != 0)
    return 0;

  for (int i = 0; i < set_size(U); i++) {
    pTest = &(arrU[i]);
    if (set_remove(U, (void **)&pTest) != 0)
      return 0; 
  }

  set_destroy(A);
  set_destroy(B);
  set_destroy(U);

  return 1;
}

static int test_intersection(Set * A, Set * B, Set * I)
{
  /* {0, 1, 2} ^ {2, 4, 6} = {2} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};
  int arrI[] = {2};
  int * pTest;

  set_init(A, match, NULL);
  set_init(B, match, NULL);
  
  for (int i = 0; i < 3; i++)
    set_insert(A, (void *)&(arrA[i]));

  for (int i = 0; i < 3; i++)
    set_insert(B, (void *)&(arrB[i]));

  if (set_intersection(I, 2, A, B) != 0)
    return 0;

  for (int i = 0; i < set_size(I); i++) {
    pTest = &(arrI[i]);
    if (set_remove(I, (void **)&pTest) != 0)
      return 0; 
  }

  set_destroy(A);
  set_destroy(B);
  set_destroy(I);

  return 1;
}

static int test_subtraction(Set * A, Set * B, Set * S)
{
  /* {0, 1, 2} - {2, 4, 6} = {0, 1} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};
  int arrS[] = {0, 1};
  int * pTest;

  set_init(A, match, NULL);
  set_init(B, match, NULL);
  
  for (int i = 0; i < 3; i++)
    set_insert(A, (void *)&(arrA[i]));

  for (int i = 0; i < 3; i++)
    set_insert(B, (void *)&(arrB[i]));

  if (set_difference(S, A, B) != 0)
    return 0;

  for (int i = 0; i < set_size(S); i++) {
    pTest = &(arrS[i]);
    if (set_remove(S, (void **)&pTest) != 0)
      return 0; 
  }

  set_destroy(A);
  set_destroy(B);
  set_destroy(S);

  return 1;
}
#endif /* CONFIG_DEBUG_SET */

/******************************************************************************/
