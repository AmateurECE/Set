/******************************************************************************
 * NAME:	    test.c
 *
 * AUTHOR:	    Ethan D. Twardy
 *
 * DESCRIPTION:	    The source file containing the tests for the API in set.c.
 *
 * CREATED:	    01/18/2018
 *
 * LAST EDITED:	    01/22/2018
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

#   define log(...) fprintf(logfile, __VA_ARGS__);
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
void printset(void *);
static set * prep_set();

static int test_create();
static int test_destroy();
static int test_remove();
static int test_insert();
static int test_isequal(set *, set *);
static int test_union(set *, set *, set *);
static int test_intersection(set *, set *, set *);
static int test_difference(set *, set *, set *);
#endif /* CONFIG_DEBUG_SET */

/******************************************************************************
 * MAIN
 ***/

#ifdef CONFIG_DEBUG_SET
int main(int argc, char * argv[])
{
  set * set1 = NULL, *set2 = NULL, *set3 = NULL;
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

  	 test_create()	? PASS"PASS"NC : FAIL"FAIL"NC,
	 test_destroy()	? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_remove()	? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_insert()	? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_isequal(set1, set2) ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_union(set1, set2, set3) ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_intersection(set1, set2, set3) ? PASS"PASS"NC : FAIL"FAIL"NC,
  	 test_difference(set1, set2, set3) ? PASS"PASS"NC : FAIL"FAIL"NC
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
  if ((group = set_create(match, free)) == NULL) {
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
 *			1 - NULL, nonnull
 *			2 - nonnull, NULL
 *			3 - nonnull, nonnull
 ***/
static int test_create()
{
  /* NULL, nonnull */
  set * group = NULL;
  if ((group = set_create(NULL, free)) != NULL)
    log_fail("test_create: 1 failed--set_create() !-> NULL\n");

  /* nonnull, NULL */
  if ((group = set_create(match, NULL)) == NULL)
    log_fail("test_create: 2 failed--set_create() -> NULL\n");
  set_destroy(&group);

  /* nonnull, nonnull */
  if ((group = set_create(match, free)) == NULL)
    log_fail("test_create: 3 failed--set_create() -> NULL\n");
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
  if ((group = set_create(match, free)) == NULL)
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
 * ARGUMENTS:	    set: (set *) -- a set to use for the tests.
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
 * ARGUMENTS:	    set: (set *) -- a set to use for the tests.
 *
 * RETURN:	    (int) -- 1 if the tests pass, 0 if they fail.
 *
 * NOTES:	    TODO: Update test_insert
 *		    Test cases:
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
 * ARGUMENTS:	    A: (set *) -- The first set to use in the tests.
 *		    B: (set *) -- The second set to use in the tests.
 *
 * RETURN:	    (int) -- 1 if the test passes, 0 otherwise.
 *
 * NOTES:	    TODO: Update test_isequal
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
 * NOTES:	    TODO: Update test_union
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
    if (set_remove(U, (const void **)&pTest) != 0)
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
 * NOTES:	    TODO: Update test_intersection
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
 * NOTES:	    TODO: Update test_difference
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
