/*
 * This file provides a cool macro to check the return values
 * without polluting your beautiful code. Few examples:
 *
 * CHECK(pipe(pipefd) != -1);
 * CHECK((pid = fork()) != -1);
 * CHECK(wait(NULL) != -1);
 *
 * In your code, just add this to use it:
 * #include "check.h"
 */

#include <stdio.h> // For fprintf, perror
#include <stdlib.h> // For exit

#define CHECK(x) \
  do { \
    if (!(x)) { \
      fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
      perror(#x); \
      exit(EXIT_FAILURE); \
    } \
  } while (0)


