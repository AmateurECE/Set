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
 * CREATED:	    05/09/17
 *
 * LAST EDITED:	    01/18/2018
 ***/

/******************************************************************************
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

/******************************************************************************
 * MACRO DEFINITIONS
 ***/

#ifdef CONFIG_DEBUG_SET
#define error_exitf(str, ...) {			\
    fprintf(stderr, str, __VA_ARGS__);		\
    printf("\n");				\
    exit(1);					\
  }
#define error_exit(str) {			\
    fprintf(stderr, str);			\
    printf("\n");				\
    exit(1);					\
  }
#endif

/******************************************************************************
 * LOCAL PROTOTYPES
 ***/

#ifdef CONFIG_DEBUG_SET
int match(const void *, const void *);
void printset(void *);

static int test_remove(Set *);
static int test_insert(Set *);
static int test_isequal(Set *, Set *);
static int test_union(Set *, Set *, Set *);
static int test_intersection(Set *, Set *, Set *);
static int test_difference(Set *, Set *, Set *);
#endif /* CONFIG_DEBUG_SET */

/******************************************************************************
 * API FUNCTIONS
 ***/

/******************************************************************************
 * FUNCTION:	    set_create
 *
 * DESCRIPTION:	    Initializes the set pointer with the parameters given.
 *
 * ARGUMENTS:	    match: (int (*)(const void *, const void *)) -- a pointer
 *			   to a user-defined function that compares two keys
 *			   and determines if they are equal. Should return 1
 *			   for equality and 0 otherwise.
 *		    destroy: (void (*)(void *)) -- a pointer to a user-defined
 *			     function that frees data held in the list.
 *
 * RETURN:	    (Set *) -- pointer to the new set, or NULL.
 *
 * NOTES:	    O(1)
 ***/
Set * set_create(int (*match)(const void *, const void *),
		 void (*destroy)(void *))
{
  Set * set = NULL;
  if ((set = malloc(sizeof(Set))) == NULL)
    return NULL;

  *set = (Set){
    .size = 0,
    .match = match,
    .destroy = destroy,
    .head = NULL,
    .tail = NULL
  };

  return set;
}

/******************************************************************************
 * FUNCTION:	    set_ismember
 *
 * DESCRIPTION:	    Determines if the key provided in 'data' is already a
 *		    member of the set.
 *
 * ARGUMENTS:	    set: (const Set *) -- the set to be operated on.
 *		    data: (const void *) -- data to check.
 *
 * RETURN:	    int -- 1 if the member is in the set, 0 if it is not, -1 if
 *		    an error has occurred.
 *
 * NOTES:	    O(n)
 *		    TODO: Fix all usage of set_ismember to reflect return value
 ***/
int set_ismember(const Set * set, const void * data)
{
  if (set == NULL || data == NULL)
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

/******************************************************************************
 * FUNCTION:	    set_insert
 *
 * DESCRIPTION:	    Inserts the 'data' into the set if it does not already
 *		    exist in the set.
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

/******************************************************************************
 * FUNCTION:	    set_remove
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

/******************************************************************************
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

/******************************************************************************
 * FUNCTION:	    set_destroy
 *
 * DESCRIPTION:	    Removes all data from the set and sets all bytes of memory
 *		    to 0. If destroy is set to NULL, does not attempt to free
 *		    the memory associated with the data in the set.
 *
 * ARGUMENTS:	    set: (Set **) -- the set to be operated on.
 *
 * RETURN:	    void.
 *
 * NOTES:	    O(n)
 ***/
void set_destroy(Set ** set)
{
  void * data;
  Member * old;

  while (set_size(*set) > 0) {
    data = (*set)->head->data;
    old = (*set)->head;
    (*set)->head = (*set)->head->next;
    
    (*set)->size--;
    free(old);
    
    if ((*set)->destroy != NULL) {
      (*set)->destroy(data);
    }
  }

  free(*set);
  *set = NULL;
}

/******************************************************************************
 * FUNCTION:	    set_union_func
 *
 * DESCRIPTION:	    Performs the union set operation and places the result in
 *		    setu.
 *
 * ARGUMENTS:	    setu: (Set *) -- will contain a pointer to the union of all
 *			    sets at the end of the call.
 *		    ...: (Set *) -- all further parameters will be cast to
 *			    pointer to Set and unioned.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn), where m is the number of sets unioned. Should always
 *		    be called by wrapper macro.
 ***/
int set_union_func(Set ** setu,  Set * sets[])
{
  if (sets[0] == NULL || setu == NULL)
    return -1;
  if (*setu == NULL) {
    if ((*setu = set_create(sets[0]->match, sets[0]->destroy)) == NULL)
      return -1;
  } else if (set_size(*setu) > 0) {
    return -1;
  }

  int i = 0;
  for (Set * set = sets[i]; set != NULL; set = sets[i++]) {
    for (Member * current = set->head; current != NULL; set_next(current)) {
      if (set_insert(*setu, (void *)current->data) < 0)
	goto error_exception;
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
 * ARGUMENTS:	    seti: (Set *) -- will contain a pointer to the intersection
 *			of all sets at the end of the call. May be NULL, or
 *			the empty set. All other values return error.
 *		    numargs: int -- contains the number of arguments passed.
 *		    ...: (Set *) -- all further parameters will be cast to
 *			 pointer to Set and intersected.
 *
 * RETURN:	    int -- 0 if computation was successful, -1 otherwise.
 *
 * NOTES:	    O(mn), where m is the number of sets passed.
 *		    TODO: set_intersection_func - Return intersecting set.
 *		    TODO: set_intersection_func - Sanitize inputs accordingly.
 *		    TODO: Fix params for all set operation functions.
 ***/
int set_intersection_func(Set ** seti, Set * sets[])
{
  if (sets[0] == NULL || seti == NULL)
    return -1;
  if (*seti == NULL) {
    if ((*seti = set_create(sets[0]->match, sets[0]->destroy)) == NULL)
      return -1;
  } else if (set_size(*seti) > 0) {
    return -1;
  }

  for (Member * current = (*sets)->head; current != NULL; set_next(current)) {
    int nonmember = 0, j = 1;
    while (sets[j] != NULL && !nonmember)
      nonmember = !set_ismember((const Set *)sets[j++], current->data);

    if (nonmember)
      continue;
    set_insert(*seti, current->data);
  }

  return 0;
}

/******************************************************************************
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
int set_difference(Set ** setd, const Set * set1, const Set * set2)
{
  if (setd == NULL || set1 == NULL || set2 == NULL)
    return -1;
  if ((*setd = set_create(set1->match, set1->destroy)) == NULL)
    return -1;

  Member * current;
  for (current = set1->head; current != NULL; set_next(current)) {
    
    if (!set_ismember(set2, current->data))
      if (set_insert(*setd, current->data) != 0)
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

/******************************************************************************
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

/******************************************************************************
 * MAIN
 ***/

/* TODO: Move testing to another file.
 * TODO: Improve format of testing (Similar to bitree)
 */

#ifdef CONFIG_DEBUG_SET
int main(int argc, char * argv[])
{
  /* STANDARD TEST. */
  Set * set = NULL;
  int * pNum = NULL;

  if ((set = set_create(match, free)) == NULL)
    error_exit("Could not allocate memory for set!");

  srand((unsigned)time(NULL));

  /* Inserting */
  printf("==== Inserting ====\n");
  while (set_size(set) < 10) {
    pNum = malloc(sizeof(int));
    *pNum = rand() % 10;
    printf("int %d @ %p", *pNum, pNum);
    int ret = set_insert(set, (void *)pNum);
    if (ret < 0) {
      printf(": FAILED\n");
      free(pNum);
    } else if (ret > 0) {
      printf(": IS A MEMBER\n");
      free(pNum);
    } else {
      printf("\n");
    }
  }

  /* Removing */
  printf("==== Removing =====\n");
  while (set_size(set) > 0) {
    pNum = NULL;
    if (set_remove(set, (void **)&pNum) < 0) {
      error_exit("Error in set_remove");
    } else {
      printf("int %d @ %p\n", *pNum, pNum);
    }
    free(pNum);
  }

  set_destroy(&set);

  assert(test_remove(set) == 1);
  assert(test_insert(set) == 1);

  Set *set2 = NULL, *set3 = NULL;

  assert(test_isequal(set, set2));
  assert(test_union(set, set2, set3));
  assert(test_intersection(set, set2, set3));
  assert(test_difference(set, set2, set3));

  return 0;
}

/******************************************************************************
 * LOCAL FUNCTIONS
 ***/

/******************************************************************************
 * FUNCTION:	    match
 *
 * DESCRIPTION:	    Used by set_create, set_insert, and the set operations.
 *
 * ARGUMENTS:	    one: (const void *) -- the first datum.
 *		    two: (const void *) -- the second datum.
 *
 * RETURN:	    (int) -- 1 if one and two are equivalent, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
int match(const void * one, const void * two)
{
  if (one == NULL && two == NULL)
    return 1;
  if (one == NULL || two == NULL)
    return 0;

  int a = *((int *)one);
  int b = *((int *)two);

  if (a == b)
    return 1;
  else
    return 0;
}

/******************************************************************************
 * FUNCTION:	    printset
 *
 * DESCRIPTION:	    Used by set_traverse, for debugging.
 *
 * ARGUMENTS:	    data: (void *) -- the data in each member of the set.
 *
 * RETURN:	    void.
 *
 * NOTES:	    none.
 ***/
void printset(void * data)
{
  int * pInt = (int *)data;
  printf("%d @ %p\n", *pInt, pInt);
}

/******************************************************************************
 * FUNCTION:	    test_remove
 *
 * DESCRIPTION:	    Tests the set_remove function.
 *
 * ARGUMENTS:	    set: (Set *) -- a set to use for the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 if the tests fail.
 *
 * NOTES:	    none.
 ***/
static int test_remove(Set * set)
{
  if ((set = set_create(match, free)) == NULL)
    return 0;
  int * pTest = malloc(sizeof(int));
  *pTest = 1;
  int ret = set_remove(set, (void **)&pTest);
  set_destroy(&set);
  free(pTest);
  return ret == -1;
}

/******************************************************************************
 * FUNCTION:	    test_insert
 *
 * DESCRIPTION:	    Tests the set_insert function.
 *
 * ARGUMENTS:	    set: (Set *) -- a set to use for the tests.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 if they fail.
 *
 * NOTES:	    none.
 ***/
static int test_insert(Set * set)
{
  if ((set = set_create(match, free)) == NULL)
    return 0;
  int * pTest = malloc(sizeof(int));
  *pTest = 1;
  if (set_insert(set, (void *)pTest) < 0)
    error_exit("There was a problem in test_insert: set_insert");

  pTest = malloc(sizeof(int));
  *pTest = 2;
  if (set_insert(set, (void *)pTest))
    error_exit("There was a problem in test_insert: set_insert");
  
  set_destroy(&set);
  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_isequal
 *
 * DESCRIPTION:	    Tests the set_isequal function.
 *
 * ARGUMENTS:	    A: (Set *) -- The first set to use in the tests.
 *		    B: (Set *) -- The second set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_isequal(Set * A, Set * B)
{
  /* {1} = {1} */
  if ((A = set_create(match, free)) == NULL)
    return 0;
  if ((B = set_create(match, free)) == NULL)
    return 0;

  int * pA = malloc(sizeof(int));
  int * pB = malloc(sizeof(int));
  *pA = 1, *pB = 1;

  if (set_insert(A, pA) != 0 || set_insert(B, pB) != 0)
    error_exit("There was a problem in test_isequal: set_insert");

  int ret = set_isequal(A, B);
  set_destroy(&A);
  set_destroy(&B);
  return ret;
}

/******************************************************************************
 * FUNCTION:	    test_union
 *
 * DESCRIPTION:	    Tests the set_union function.
 *
 * ARGUMENTS:	    A: (Set *) -- The first set to use in the tests.
 *		    B: (Set *) -- The second set to use in the tests.
 *		    C: (Set *) -- The third set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_union(Set * A, Set * B, Set * U)
{
  /* {0, 1, 2} U {2, 4, 6} = {0, 1, 2, 4, 6} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};
  int arrU[] = {0, 1, 2, 4, 6};
  int * pTest;

  if ((A = set_create(match, NULL)) == NULL)
    return 0;
  if ((B = set_create(match, NULL)) == NULL)
    return 0;
  
  for (int i = 0; i < 3; i++)
    set_insert(A, (void *)&(arrA[i]));

  for (int i = 0; i < 3; i++)
    set_insert(B, (void *)&(arrB[i]));

  if (set_union(&U, A, B) != 0)
    return 0;

  for (int i = 0; i < set_size(U); i++) {
    pTest = &(arrU[i]);
    if (set_remove(U, (void **)&pTest) != 0)
      return 0; 
  }

  set_destroy(&A);
  set_destroy(&B);
  set_destroy(&U);

  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_intersection
 *
 * DESCRIPTION:	    Tests the set_intersection function.
 *
 * ARGUMENTS:	    A: (Set *) -- the first set to use in the tests.
 *		    B: (Set *) -- the second set to use in the tests.
 *		    C: (Set *) -- the third set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_intersection(Set * A, Set * B, Set * I)
{
  /* {0, 1, 2} ^ {2, 4, 6} = {2} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};

  if ((A = set_create(match, NULL)) == NULL)
    return 0;
  if ((B = set_create(match, NULL)) == NULL)
    return 0;
  /* Don't need to initialize I because set_intersection does that.
   */

  for (int i = 0; i < 3; i++)
    set_insert(A, (void *)&(arrA[i]));

  for (int i = 0; i < 3; i++)
    set_insert(B, (void *)&(arrB[i]));

  if (set_intersection(&I, A, B) != 0)
    error_exit("Failure in set_intersection.");

  /* set_traverse(I, printset); */

  if (set_size(I) != 1 || *((int *)I->head->data) != 2)
    error_exit("Failure in set_intersection.");

  set_destroy(&A);
  set_destroy(&B);
  set_destroy(&I);

  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_difference
 *
 * DESCRIPTION:	    Tests the set_difference function.
 *
 * ARGUMENTS:	    A: (Set *) -- The first set to use in the tests.
 *		    B: (Set *) -- The second set to use in the tests.
 *		    C: (Set *) -- The third set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_difference(Set * A, Set * B, Set * S)
{
  /* {0, 1, 2} - {2, 4, 6} = {0, 1} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};

  if ((A = set_create(match, NULL)) == NULL)
    return 0;
  if ((B = set_create(match, NULL)) == NULL)
    return 0;
  
  for (int i = 0; i < 3; i++)
    set_insert(A, (void *)&(arrA[i]));

  for (int i = 0; i < 3; i++)
    set_insert(B, (void *)&(arrB[i]));

  if (set_difference(&S, A, B) != 0)
    return 0;

  /* set_traverse(S, printset); */

  set_destroy(&A);
  set_destroy(&B);
  set_destroy(&S);

  return 1;
}
#endif /* CONFIG_DEBUG_SET */

/*****************************************************************************/
