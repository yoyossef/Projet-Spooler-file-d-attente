#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include "check.h"

#define MONEOF -1
#define LOCK_FILE "verrou"
#define SPOOL "spool"

char *concat(char *a, char *b);

void deposer(char ** fichiers, int nb);