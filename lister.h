#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include "check.h"

#define MONEOF -1
#define LOCK_FILE	"verrou"
#define SPOOL "spool"

char * concat(char * a, char * b);

void lister(int lflag, int uflag, char * user);

