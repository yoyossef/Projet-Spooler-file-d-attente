/**
 * \file lister.c
 * \brief Programme qui liste les jobs dans le spool
 * \author ROSTAQI Yossef et PENG Zhigang
 * \date 7 Novembre 2017
 *
 *
 */

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

char * concat(char * a, char * b){
    char * dest = (char *) malloc(strlen(a) + strlen(b) + 1);
    strcpy(dest, a);
    strcat(dest, b);
    return dest;
}

/**
 * \fn void lister(int lflag, int uflag, char * user)
 * \brief Liste les jobs dans le spool sur la sortie standard
 *
 * \param int lflag, flag de l'option i
 * \param int uflag, flag de l'option u
 * \param char * user, nom de l'utilisateur si l'option u a été activée
 * \return void
 */

void lister(int lflag, int uflag, char * user){
	int lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
    while (lockf(lfp, F_TEST, 0) != 0)
    {
        sleep(1); /* can not lock */
    }
    lockf(lfp, F_LOCK, 0);
	FILE * fd;
	struct stat * stbuf = malloc(sizeof(struct stat));
	CHECK(stat(SPOOL, stbuf) != MONEOF);
	DIR * dp = NULL;
	char * chemin_spool = NULL;
	if(getenv("PROJETSE") != NULL){
		CHECK(stat(getenv("PROJETSE"), stbuf) != MONEOF);
		dp = opendir(getenv("PROJETSE"));
		chemin_spool = concat(getenv("PROJETSE"), "/");		
    }
    else{
		CHECK(stat(SPOOL, stbuf) != MONEOF);
		dp = opendir(SPOOL);
		chemin_spool = concat(SPOOL, "/");		
    }
	struct dirent * d;
	char login[20];
	char nomfich[50];
	char jour[5];
	char mois [5];
	int numjour, heure, minute, seconde, annee;
	// char * chemin_spool = concat(SPOOL,"/");

	if(lflag || uflag){
		if(stbuf -> st_uid != getuid()){
			fprintf(stderr, "lister: Option -l or -u are restricted to spool owner \n");
			free(stbuf);
			exit(1);
		}
	}
	while((d = readdir(dp)) != NULL){
		//On ignore les fichiers . et .. ainsi que les fichiers commençant par j
		if(strcmp (d -> d_name, ".") != 0 && strcmp(d -> d_name, "..") != 0 && d -> d_name[0] != 'j'){
			char * chemin = concat(chemin_spool, d->d_name);			
			char * id = (char *) malloc(strlen(d->d_name) + 1);
			//On copie l'id XXXXXX dans id
			strcpy(id, &d->d_name[strlen(d->d_name) - 6]);
			fd = fopen(chemin, "r");
			CHECK(fd != NULL);
			fscanf(fd, "%s %*d %s %s %s  %d %d%*c%d%*c%d %d", login, nomfich, jour, mois, &numjour, &heure, &minute, &seconde, &annee);
			if(lflag){
				if(!uflag || (uflag && (strcmp(user, login) == 0)))
					printf("%s %s %s\t%s %s  %d %d:%d:%d %d\n", id, login, nomfich, jour, mois, numjour, heure, minute, seconde, annee);
				else{
					fclose(fd);
					free(chemin);
					free(id);
					continue;
				}
			}
			else if(uflag){
				if(strcmp(user, login) == 0)
					printf("%s %s\t\t%s %s  %d %d:%d:%d %d\n", id, login, jour, mois, numjour, heure, minute, seconde, annee);
				else{
					fclose(fd);
					free(chemin);
					free(id);
					continue;
				}
			}
			else
				printf("%s %s\t\t%s %s  %d %d:%d:%d %d\n", id, login, jour, mois, numjour, heure, minute, seconde, annee);			
			fclose(fd);
			free(chemin);
			free(id);
		}
	}
	free(chemin_spool);
	closedir(dp);
	free(stbuf);
	lockf(lfp, F_ULOCK, 0);
    close(lfp);
}

int main (int argc, char ** argv){
	int c, lflag = 0, uflag = 0;
	extern char * optarg;
	char * input;
	extern int optind;
	while ((c = getopt (argc, argv, "lu:")) != MONEOF){
		switch (c){
			case 'l' :
			 	lflag++;
			 	break;

			case 'u' :
				input = optarg;
			 	uflag++;
			 	break;

			default :
			 	fprintf(stderr, "Usage : lister -l -u <user> \n");
			 	exit (1);
			 	break;
		}
	}
	lister(lflag, uflag, input);
}