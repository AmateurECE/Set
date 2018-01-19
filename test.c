/******************************************************************************
 * NAME:	    test.c
 *
 * AUTHOR:	    Ethan D. Twardy
 *
 * DESCRIPTION:	    The source file containing the tests for the API in set.c.
 *
 * CREATED:	    01/18/2018
 *
 * LAST EDITED:	    01/18/2018
 ***/

/******************************************************************************
 * INCLUDES
 ***/

#include <stdio.h>
#include <stdlib.h>

#ifdef CONFIG_DEBUG_SET
#include <time.h>
#include <assert.h>
#endif /* CONFIG_DEBUG_SET */

#include "set.h"

/******************************************************************************
 * MACRO DEFINITIONS
 ***/

#ifdef CONFIG_DEBUG_SET
#define FAIL "\033[1;31m"

/* This is specific to my terminal, I think. But since it only affects the
 * colors of the test output, it doesn't really matter
 */
#ifdef __APPLE__
#   define PASS "\033[1;32m"
#else
#   define PASS "\033[1;39m"
#endif

#define NC	"\033[0m"

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

static int test_remove(set *);
static int test_insert(set *);
static int test_isequal(set *, set *);
static int test_union(set *, set *, set *);
static int test_intersection(set *, set *, set *);
static int test_difference(set *, set *, set *);
#endif /* CONFIG_DEBUG_SET */

/******************************************************************************
 * MAIN
 ***/

/* TODO: Improve format of testing (Similar to bitree)
 */

#ifdef CONFIG_DEBUG_SET
int main(int argc, char * argv[])
{
  /* STANDARD TEST. */
  set * set1 = NULL, *set2 = NULL, *set3 = NULL;
  srand((unsigned)time(NULL));

  printf("Test remove (set_remove):\t\t%s\n"
	 "Test insert (set_insert):\t\t%s\n"
	 "Test isequal (set_isequal):\t\t%s\n"
	 "Test union (set_union):\t\t\t%s\n"
	 "Test intersection (set_intersection):\t%s\n"
	 "Test difference (set_difference):\t%s\n",

	 test_remove(set1) ? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_insert(set1) ? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_isequal(set1, set2) ? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_union(set1, set2, set3) ? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_intersection(set1, set2, set3) ? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_difference(set1, set2, set3) ? PASS"PASS"NC : FAIL"FAIL"NC
	 );

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
 * ARGUMENTS:	    set: (set *) -- a set to use for the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 if the tests fail.
 *
 * NOTES:	    none.
 ***/
static int test_remove(set * set)
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
 * ARGUMENTS:	    set: (set *) -- a set to use for the tests.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 if they fail.
 *
 * NOTES:	    none.
 ***/
static int test_insert(set * set)
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
 * ARGUMENTS:	    A: (set *) -- The first set to use in the tests.
 *		    B: (set *) -- The second set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_isequal(set * A, set * B)
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
 * ARGUMENTS:	    A: (set *) -- The first set to use in the tests.
 *		    B: (set *) -- The second set to use in the tests.
 *		    C: (set *) -- The third set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_union(set * A, set * B, set * U)
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
 * ARGUMENTS:	    A: (set *) -- the first set to use in the tests.
 *		    B: (set *) -- the second set to use in the tests.
 *		    C: (set *) -- the third set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_intersection(set * A, set * B, set * I)
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
 * ARGUMENTS:	    A: (set *) -- The first set to use in the tests.
 *		    B: (set *) -- The second set to use in the tests.
 *		    C: (set *) -- The third set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 otherwise.
 *
 * NOTES:	    none.
 ***/
static int test_difference(set * A, set * B, set * S)
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
