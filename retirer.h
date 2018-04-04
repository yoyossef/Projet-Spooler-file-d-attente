#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "check.h"

#define LOCK_FILE "verrou"
#define SPOOL "spool"
#define MONEOF -1

char *concat(char *a, char *b);

void retirer(char **id, int nb);

