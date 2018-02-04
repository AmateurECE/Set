/******************************************************************************
 * NAME:	    test.c
 *
 * AUTHOR:	    Ethan D. Twardy
 *
 * DESCRIPTION:	    The source file containing the tests for the API in set.c.
 *
 * CREATED:	    01/18/2018
 *
 * LAST EDITED:	    02/02/2018
 ***/

/******************************************************************************
 * INCLUDES
 ***/

#ifdef CONFIG_DEBUG_SET
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "set.h"
#endif /* CONFIG_DEBUG_SET */

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

#ifdef CONFIG_TEST_LOG
#   ifndef CONFIG_LOG_FILENAME
#	define CONFIG_LOG_FILENAME "./log.txt"
#   endif

#   define log(...) fprintf(logfile, __VA_ARGS__)
#   define log_fail(...) {			\
    log(__VA_ARGS__);				\
    failures++;					\
    return 0;					\
  }

#   define CONFIG_LOG_PREAMBLE			\
  "MODULE - TEST LOG\n"				\
  "%s\n"					\
  "====================\n\n"
#else
#   define log(...)
#   define log_fail(...) { return 0; }
#endif /* CONFIG_TEST_LOG */
#endif /* CONFIG_DEBUG_SET */

/******************************************************************************
 * STATIC VARIABLES
 ***/

#ifdef CONFIG_TEST_LOG
static FILE * logfile = NULL;
#endif

static int failures = 0;

/******************************************************************************
 * LOCAL PROTOTYPES
 ***/

#ifdef CONFIG_DEBUG_SET
int match(const void *, const void *);
void * copy(const void *);
void printset(void *);
static set * prep_set();

static int test_create();
static int test_destroy();
static int test_remove();
static int test_insert();
static int test_isequal();
static int test_union();
static int test_intersection(set *, set *, set *);
static int test_difference();
#endif /* CONFIG_DEBUG_SET */

/******************************************************************************
 * MAIN
 ***/

#ifdef CONFIG_DEBUG_SET
int main(int argc, char * argv[])
{
  srand((unsigned)time(NULL));

#ifdef CONFIG_TEST_LOG
  /* Initialize log */
  printf("Initializing log in '"CONFIG_LOG_FILENAME"'...\n");
  logfile = fopen(CONFIG_LOG_FILENAME, "w");
  if (logfile == NULL)
    error_exitf("Could not initialize log file: %s\n", strerror(errno));
  char * buff = NULL;
  if ((buff = malloc(64)) == NULL)
    error_exit("Could not allocate space for time buffer.");
  time_t raw;
  time(&raw);
  strftime(buff, 63, "%+", localtime(&raw));
  log(CONFIG_LOG_PREAMBLE, buff);
  free(buff);
#endif

  printf("Test create (set_create):\t\t%s\n"
	 "Test destroy (set_destroy):\t\t%s\n"
  	 "Test remove (set_remove):\t\t%s\n"
  	 "Test insert (set_insert):\t\t%s\n"
  	 "Test isequal (set_isequal):\t\t%s\n"
  	 "Test union (set_union):\t\t\t%s\n"
  	 "Test intersection (set_intersection):\t%s\n"
  	 "Test difference (set_difference):\t%s\n",

  	 test_create()	    ? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_destroy()	    ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_remove()	    ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_insert()	    ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_isequal()	    ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_union()	    ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 /* test_intersection(set1, set2, set3) */ 0
	 ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_difference()  ? PASS"PASS"NC : FAIL"FAIL"NC
  	 );


#ifdef CONFIG_TEST_LOG
  fprintf(logfile, "END OF LOG\n");
  printf("Closing log file...\n");
  fclose(logfile);
#endif

  return failures;
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
 * FUNCTION:	    copy
 *
 * DESCRIPTION:	    This function (meant to be user-defined in practice) copies
 *		    the user data passed in `data' and returns a pointer to it.
 *		    If there is an error, this function returns NULL, and the
 *		    set API will return immediately with an error code.
 *
 * ARGUMENTS:	    data: (const void *) -- the data to copy.
 *
 * RETURN:	    void * -- pointer to the copied data, or NULL.
 *
 * NOTES:	    none.
 ***/
void * copy(const void * data)
{
  int * new = NULL;
  if ((new = malloc(sizeof(int))) == NULL)
    return NULL;
  *new = *((int *)data);
  return new;
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
#ifdef CONFIG_TEST_LOG
  fprintf(logfile, "\t%d @ %p\n", *pInt, pInt);
#else
  printf("\t%d @ %p\n", *pInt, pInt);
#endif
}

/******************************************************************************
 * FUNCTION:	    prep_set
 *
 * DESCRIPTION:	    Prepares a generic set structure for testing.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    set * -- a pointer to a new set, or NULL if an error has
 *		    occurred.
 *
 * NOTES:	    none.
 ***/
static set * prep_set()
{
  set * group = NULL;
  if ((group = set_create(match, copy, free)) == NULL) {
    log("prep_set: set_create() -> NULL");
    return NULL;
  }

  int * pNum = NULL;
  int ret = 0;
  while (set_size(group) < 3) {
    if ((pNum = malloc(sizeof(int))) == NULL) {
      log("prep_test: malloc -> NULL");
      goto error_except;
    }
    *pNum = rand() % 10;
    if ((ret = set_insert(group, (void *)pNum)) == -1) {
      log("prep_set: set_insert() -> %d\n", ret);
      goto error_except;
    } else if (ret == 1) {
      free(pNum);
    }
  }

  return group;

 error_except: {
    set_destroy(&group);
    return NULL;
  }
}

/******************************************************************************
 * FUNCTION:	    test_create
 *
 * DESCRIPTION:	    Test the set_create() function.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    int -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    Test cases:
 *			1 - NULL, nonnull, nonnull
 *			2 - nonnull, NULL, nonnull
 *			3 - nonnull, nonnull, NULL
 *			4 - nonnull, nonnull, nonnull
 ***/
static int test_create()
{
  /* NULL, nonnull, nonull */
  set * group = NULL;
  if ((group = set_create(NULL, copy, free)) != NULL)
    log_fail("test_create: 1 failed--set_create() !-> NULL\n");

  /* nonnull, NULL, nonnull */
  if ((group = set_create(match, NULL, free)) == NULL)
    log_fail("test_create: 2 failed--set_create() -> NULL\n");
  set_destroy(&group);

  /* nonnull, nonnull, NULL */
  if ((group = set_create(match, copy, NULL)) == NULL)
    log_fail("test_create: 3 failed--set_create() -> NULL\n");
  set_destroy(&group);

  /* nonnull, nonnull, nonnull */
  if ((group = set_create(match, copy, free)) == NULL)
    log_fail("test_create: 4 failed--set_create() -> NULL\n");
  set_destroy(&group);

  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_destroy
 *
 * DESCRIPTION:	    Test the set_destroy() function.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    int -- 1 if the tests pass, 0 otherwise.
 *
 * NOTES:	    Test cases:
 *			1 - NULL
 *			2 - *NULL
 *			3 - Set is empty
 *			4 - Set is not empty
 ***/
static int test_destroy()
{
  /* set_destroy returns void, so our only concern with these tests is
   * preventing a program crash. */
  set * group = NULL;
  set_destroy(NULL); /* NULL */
  set_destroy(&group); /* *NULL */

  /* Set is empty */
  if ((group = set_create(match, copy, free)) == NULL)
    log_fail("test_destroy: 3 failed--set_create() -> NULL\n");
  set_destroy(&group);

  /* Set is not empty */
  if ((group = prep_set()) == NULL)
    log_fail("test_destroy: 4 failed--prep_set() -> NULL\n");
  set_destroy(&group);

  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_remove
 *
 * DESCRIPTION:	    Tests the set_remove function.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 if the tests fail.
 *
 * NOTES:	    Test cases:
 *			1 - NULL, nonnull
 *			2 - nonnull, NULL
 *			3 - data is in the set
 *			4 - data is not in the set
 ***/
static int test_remove()
{
  /* NULL, nonnull */
  set * group = NULL;
  int * pNum = NULL;
  int ret;
  if ((group = prep_set()) == NULL)
    log_fail("test_remove: 1 failed--prep_set returned NULL.\n");
  if (!(ret = set_remove(NULL, group->head->data)))
    log_fail("test_remove: 1 failed--set_remove() -> %d\n", ret);

  /* nonnull, NULL */
  if (!(ret = set_remove(group, NULL)))
    log_fail("test_remove: 2 failed--set_remove() -> %d\n", ret);

  /* data is in the set */
  if ((ret = set_remove(group, (const void **)&(group->head->data))))
    log_fail("test_remove: 3 failed--set_remove() -> %d\n", ret);

  /* data is not in the set */
  if ((pNum = malloc(sizeof(int))) == NULL)
    log_fail("test_remove: 4 failed--malloc() -> NULL\n");
  *pNum = 11;
  if (!(ret = set_remove(group, (void *)&pNum)))
    log_fail("test_remove: 4 failed--set_remove() -> %d\n", ret);

  set_destroy(&group);
  free(pNum);
  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_insert
 *
 * DESCRIPTION:	    Tests the set_insert function.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 if they fail.
 *
 * NOTES:	    Test cases:
 *			1 - NULL, nonnull
 *			2 - nonnull, NULL
 *			3 - Otherwise
 ***/
static int test_insert()
{
  /* NULL, nonnull */
  set * group = NULL;
  int * pNum = NULL;
  if ((group = prep_set()) == NULL)
    log_fail("test_insert: 1 failed--prep_set() -> NULL\n");
  if ((pNum = malloc(sizeof(int))) == NULL)
    log_fail("test_insert: 1 failed--malloc() -> NULL\n");
  *pNum = 11; /* Numbers greater than ten will not appear in the set */
  if (!set_insert(NULL, pNum))
    log_fail("test_insert: 1 failed--set_insert() -> 0\n");

  /* nonnull, NULL */
  if (!set_insert(group, NULL))
    log_fail("test_insert: 2 failed--set_insert() -> 0\n");

  /* Otherwise */
  if (set_insert(group, pNum))
    log_fail("test_insert: 3 failed--set_insert() !-> 0\n");

  set_destroy(&group);
  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_isequal
 *
 * DESCRIPTION:	    Tests the set_isequal function.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    Test cases:
 *			1 - NULL, nonnull
 *			2 - nonnull, NULL
 *			3 - (0), set
 *			4 - set, (0)
 *			5 - set, !set
 *			6 - set, set
 ***/
static int test_isequal()
{
  set *A = NULL, *B = NULL;
  if ((A = prep_set()) == NULL)
    log_fail("test_isequal: 1 failed--prep_set() -> NULL\n");

  /* NULL, nonnull */
  if (set_isequal(B, A))
    log_fail("test_isequal: 1 failed--set_isequal() -> 1\n");

  /* nonnull, NULL */
  if (set_isequal(A, B))
    log_fail("test_isequal: 2 failed--set_isequal() -> 1\n");

  /* (0), set */
  if ((B = set_create(match, copy, free)) == NULL)
    log_fail("test_isequal: 3 failed--set_create() -> NULL\n");
  if (set_isequal(B, A))
    log_fail("test_isequal: 3 failed--set_isequal() -> 1\n");

  /* set, (0) */
  if (set_isequal(A, B))
    log_fail("test_isequal: 4 failed--set_isequal() -> 1\n");

  /* set, !set */
  set_destroy(&B);
  if ((B = prep_set()) == NULL)
    log_fail("test_isequal: 5 failed--prep_set() -> NULL\n");
  if (set_isequal(A, B))
    log_fail("test_isequal: 5 failed--set_isequal() -> 1\n");

  /* set, set */
  if (set_size(A) != 3 || set_size(B) != 3)
    log_fail("test_isequal: 6 failed--set sizes are not the same\n");
  /* Make the sets the same */
  *((int *)A->head->data) = *((int *)B->head->data);
  *((int *)A->head->next->data) = *((int *)B->head->next->data);
  *((int *)A->tail->data) = *((int *)B->tail->data);
  if (!set_isequal(A, B))
    log_fail("test_isequal: 6 failed--set_isequal() -> 0\n");

  set_destroy(&A);
  set_destroy(&B);
  return 1;
}

/******************************************************************************
 * FUNCTION:	    test_union
 *
 * DESCRIPTION:	    Tests the set_union function.
 *
 * ARGUMENTS:	    none.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    Test cases:
 *			1 - NULL, set1, set2
 *			2 - setu, NULL
 *			3 - setu, set1
 *			4 - setu, set1, set2
 *			5 - setu, set1, set2, set3, set4
 *			6 - setu, set1, set2 (deterministic)
 ***/
static int test_union()
{
  /* NULL, set1, set2 */
  set *setu = NULL, *set1 = NULL, *set2 = NULL, *set3 = NULL, *set4 = NULL;
  if ((set1 = prep_set()) == NULL || (set2 = prep_set()) == NULL)
    log_fail("test_union: 1 failed--prep_set() -> NULL\n");
  if (!set_union(NULL, set1, set2))
    log_fail("test_union: 1 failed--set_union() -> 0\n");

  /* setu, NULL */
  if (!set_union(&setu, NULL))
    log_fail("test_union: 2 failed--set_union() -> 0\n");

  /* setu, set1 */
  if (set_union(&setu, set1))
    log_fail("test_union: 3 failed--set_union() -> -1\n");

  /* setu, set1, set2 */
  set_destroy(&setu);
  if (set_union(&setu, set1, set2))
    log_fail("test_union: 4 failed--set_union() -> -1\n");

  /* setu, set1, set2, set3, set4 */
  set_destroy(&setu);
  if ((set3 = prep_set()) == NULL || (set4 = prep_set()) == NULL)
    log_fail("test_union: 5 failed--set_union() -> NULL\n");
  if (set_union(&setu, set1, set2, set3, set4))
    log_fail("test_union: 5 failed--set_union() -> -1\n");

  /* setu, set1, set2 (deterministic) */
  set_destroy(&setu);
  set_destroy(&set1);
  set_destroy(&set2);
  set_destroy(&set3);
  if ((set1 = set_create(match, copy, free)) == NULL)
    log_fail("test_union: 6 failed--set_create() -> NULL\n");
  if ((set2 = set_create(match, copy, free)) == NULL)
    log_fail("test_union: 6 failed--set_create() -> NULL\n");
  if ((set3 = set_create(match, copy, free)) == NULL)
    log_fail("test_union: 6 failed--set_create() -> NULL\n");

  /* Prepare the test sets:
   *	set1 and set2 are given to set_union()
   *	set3 is compared to setu using set_isequal()
   */
  int ent1[] = {0, 4, 6}, ent2[] = {1, 5}, entu[] = {0, 1, 4, 5, 6}, iter = 0;
  while (iter < 3) {
    int *pArr = NULL, size = 0;
    set * pSet = NULL;
    switch (iter) {
    case 0:
      pArr = ent1, size = 3, pSet = set1;
      break;
    case 1:
      pArr = ent2, size = 2, pSet = set2;
      break;
    case 2:
      pArr = entu, size = 5, pSet = set3;
      break;
    default:
      goto increment; /* Unreachable code */
    }

    /* Populate the set */
    int * pNum = NULL;
    for (int i = 0; i < size; i++) {
      if ((pNum = malloc(sizeof(int))) == NULL)
	log_fail("test_union: 6 failed--malloc() -> NULL\n");
      *pNum = pArr[i];
      if (set_insert(pSet, pNum))
	log_fail("test_union: 6 failed--set_insert() -> -1\n");
    }

  increment:
    iter++;
  }

  if (set_union(&setu, set1, set2))
    log_fail("test_union: 6 failed--set_union() -> -1\n");
  if (!set_isequal(setu, set3)) {
    log("test_union: 6 failed--setu and set3 are not the same:\n");
    log("setu:\n");
#ifdef CONFIG_TEST_LOG
    set_traverse(setu, printset); /* Prints to log automatically */
#endif
    log("set3:\n");
#ifdef CONFIG_TEST_LOG
    set_traverse(set3, printset); /* Prints to log automatically */
#endif
    log_fail("...end test 6 failure report.\n");
  }
  /* END TEST 6 */

  set_destroy(&setu);
  set_destroy(&set1);
  set_destroy(&set2);
  set_destroy(&set3);
  set_destroy(&set4);
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
 * NOTES:	    TODO: Update test_intersection
 ***/
static int test_intersection(set * A, set * B, set * I)
{
  /* {0, 1, 2} ^ {2, 4, 6} = {2} */
  int arrA[] = {0, 1, 2};
  int arrB[] = {2, 4, 6};

  if ((A = set_create(match, NULL, NULL)) == NULL)
    return 0;
  if ((B = set_create(match, NULL, NULL)) == NULL)
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
 * ARGUMENTS:	    none.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 otherwise.
 *
 * NOTES:	    Test cases:
 *			1 - NULL, set, set
 *			2 - nonnull, NULL, nonnull
 *			3 - nonnull, nonnull, NULL
 *			4 - nonnull, set, set (deterministic)
 *			5 - nonnull, set, set (null result)
 *			6 - nonnull, set, set (heuristic)
 ***/
static int test_difference()
{
  /* NULL, set, set */
  set *set1 = NULL, *set2 = NULL, *set3 = NULL, *setd = NULL;;
  if ((set1 = prep_set()) == NULL || (set2 = prep_set()) == NULL)
    log_fail("test_difference: 1 failed--prep_set() -> NULL\n");
  if (!set_difference(NULL, set1, set2))
    log_fail("test_difference: 1 failed--set_difference() -> 0\n");

  /* nonnull, NULL, set */
  if (!set_difference(&setd, NULL, set1))
    log_fail("test_difference: 2 failed--set_difference() -> 0\n");

  /* nonnull, set, NULL */
  if (!set_difference(&setd, set1, NULL))
    log_fail("test_difference: 3 failed--set_difference() -> 0\n");

  /* nonnull, set, set (deterministic) */
  set_destroy(&set1);
  set_destroy(&set2);
  if ((set1 = set_create(match, copy, free)) == NULL
      || (set2 = set_create(match, copy, free)) == NULL
      || (set3 = set_create(match, copy, free)) == NULL)
    log_fail("test_difference: 4 failed--set_create() -> NULL\n");

  int arr1[] = {1, 2, 3, 4}, arr2[] = {2, 4, 6, 8},
    arrd[] = {1, 3}, *pArr = NULL, size = 0, i = 0, *pNum = NULL;
  set * pSet = NULL;
  /* Load up sets */
  while (i < 3) {
    switch (i) {
    case 1:
      pArr = arr1, size = 4, pSet = set1;
      break;
    case 2:
      pArr = arr2, size = 4, pSet = set2;
      break;
    case 3:
      pArr = arrd, size = 2, pSet = set3;
      break;
    default:
      goto break_loop; /* Unreachable code */
    }

    for (int j = 0; j < size; j++) {
      if ((pNum = malloc(sizeof(int))) == NULL)
	log_fail("test_difference: 4 failed--malloc() -> NULL\n");
      *pNum = pArr[j];
      if (set_insert(pSet, pNum))
	log_fail("test_difference: 4 failed--set_insert() !-> 0\n");
    }

    i++;
    continue;
  break_loop:
    break;
  }

  if (set_difference(&setd, set1, set2))
    log_fail("test_difference: 4 failed--set_insert() !-> 0\n");
  if (!set_isequal(setd, set3))
    log_fail("test_difference: 4 failed--setd and set3 are not the same\n");
  set_destroy(&set1);
  set_destroy(&set2);
  set_destroy(&set3);
  set_destroy(&setd);
  /* end test 4 */

  /* nonnull, set, set (null result) */
  if ((set1 = set_create(match, copy, free)) == NULL
      || (set2 = set_create(match, copy, free)) == NULL)
    log_fail("test_difference: 5 failed--set_create() -> NULL\n");

  for (pSet = set1; pSet != NULL; pSet = (pSet == set1 ? set2 : NULL)) {
    for (int j = 0; j < 4; j++) {
      if ((pNum = malloc(sizeof(int))) == NULL)
	log_fail("test_difference: 5 failed--malloc() -> NULL\n");
      *pNum = arr1[j];
      if (set_insert(pSet, pNum))
	log_fail("test_difference: 5 failed--set_insert() !-> 0\n");
    }
  }

  if (set_difference(&setd, set1, set2))
    log_fail("test_difference: 5 failed--set_difference() !-> 0\n");
  if (set_size(setd) != 0)
    log_fail("test_difference: 5 failed--setd is not empty.\n");
  set_destroy(&set1);
  set_destroy(&set2);
  set_destroy(&setd);

  /* nonnull, set, set (heuristic) */
  if ((set1 = prep_set()) == NULL
      || (set2 = prep_set()) == NULL)
    log_fail("test_difference: 6 failed--prep_set() -> NULL\n");
  if (set_difference(&setd, set1, set2))
    log_fail("test_difference: 6 failed--set_difference() !-> 0\n");
  if (setd == NULL)
    log_fail("test_difference: 6 failed--setd is NULL\n");
  set_destroy(&set1);
  set_destroy(&set2);
  set_destroy(&setd);

  return 1;
}
#endif /* CONFIG_DEBUG_SET */

/*****************************************************************************/
