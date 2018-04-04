#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include "check.h"

#define MONEOF -1
#define LOCK_FILE "verrou"
#define SPOOL "spool"

void print_usage(void);

char *concat(char *a, char *b);

void init_daemon(void);

void demon(int dflag, int iflag, int delai, char *fichlog);

