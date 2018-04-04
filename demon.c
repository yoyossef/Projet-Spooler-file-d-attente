/**
 * \file demon.c
 * \brief Programme qui crée un démon et qui appliquent gzip sur les jobs du spool
 * \author ROSTAQI Yossef et PENG Zhigang
 * \date 7 Novembre 2017
 *
 *
 */

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


/**
 * \fn void print_usage(void)
 * \brief Fonction affichant l'utilisation de l'exécutable.
 *
 */

void print_usage(void)
{
	fprintf(stderr, "Usage : demon -dfi <delai> fichier_sortie \n");
	exit(1);
}

/**
 * \fn char *concat(char *a, char *b)
 * \brief Fonction de concaténation de deux chaînes de caractères pour
 * alléger le programme.
 *
 * \param char * a
 * \param char * b
 * \return le pointeur sur la chaîne de caractères contenant a suivi de b.
 * \pre a et b ne peuvent être null
 */

char *concat(char *a, char *b)
{
	char *dest = (char *)malloc(strlen(a) + strlen(b) + 1);
	strcpy(dest, a);
	strcat(dest, b);
	return dest;
}

/**
 * \fn void init_daemon(void)
 * \brief Crée le démon, sert à passer le processus en arrière plan.
 * 
 * \return void
 */

void init_daemon(void)
{
	int pid;
	pid = fork();
	if (pid > 0)
		exit(0); //On tue le père
	else if (pid < 0)
		exit(1); //Cas d'erreur de fork
	umask(0);
	setsid();
	return;
}

/**
 * \fn void demon(int dflag, int iflag, int delai, char * fichlog)
 * \brief Vérifie périodiquement le spool pour traiter les jobs déposés,
 * traiter un job = appliquer gzip dessus. Ecrit des informations sur
 * chaque job traité dans le fichier fichlog (il le crée s'il n'existe pas).
 *
 * \param int dflag
 * \param int iflag
 * \param int delai
 * \param char * fichlog
 * \pre le dossier spool doit déjà exister
 * \return void
 */

void demon(int dflag, int iflag, int delai, char *fichlog)
{
	int compteur = 0;
	if (!dflag){
		close(STDOUT_FILENO);
		close(STDIN_FILENO);
		close(STDERR_FILENO);
	}

	while (1)
	{
		int lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
		while (lockf(lfp, F_TEST, 0) != 0)
		{
			sleep(1); /* can not lock */
		}
		lockf(lfp, F_LOCK, 0);
		pid_t p1;
		time_t mytime;
		char login[20];
		char nomfich[50];
		char jour[5];
		char mois[5];
		int taille, numjour, heure, minute, seconde, annee, status;
		struct dirent *d;
		char *chemin_spool = NULL;
		DIR *dp = NULL;
		if (getenv("PROJETSE") != NULL)
		{
			chemin_spool = concat(getenv("PROJETSE"), "/");
			dp = opendir(getenv("PROJETSE"));
		}
		else
		{
			chemin_spool = concat(SPOOL, "/");
			dp = opendir(SPOOL);
		}
		//On lit le spool jusqu'à la fin
		while ((d = readdir(dp)) != NULL)
		{
			//On ouvre le log file
			FILE *log;
			FILE *dfich;
			if (compteur > 0)
				CHECK((log = fopen(fichlog, "a")) != NULL);
			else
			{
				CHECK((log = fopen(fichlog, "w")) != NULL);
				mytime = time(NULL);
				//Pour écrire cette ligne qu'au lancement du spool
				fprintf(log, "Starting at %s", ctime(&mytime));
				compteur++;
			}
			if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0 && d->d_name[0] != 'd')
			{
				char *chemin_j = concat(chemin_spool, d->d_name);
				char *tmp = concat("d", &d->d_name[strlen(d->d_name) - 6]);
				char *chemin_d = concat(chemin_spool, tmp);
				struct stat *stbuf = malloc(sizeof(struct stat));
				dfich = fopen(chemin_d, "r");
				uid_t real;

				//On enregistre la date de dépot à partir du fichier d
				//pour l'écriredans le fichier log
				fscanf(dfich, "%s %d %s %s %s  %d %d%*c%d%*c%d %d %d", login,
					   &taille, nomfich, jour, mois, &numjour, &heure, &minute,
					   &seconde, &annee, &real);

				char * tmp2 = concat("./", &chemin_j[strlen(chemin_j) - 7]);
				uid_t effective = geteuid();

				//on déplace le job dans le rep courant
				rename(chemin_j, tmp2);

				//Si l'option d est activée, on affiche des infos de debogage
				if (dflag)
					printf("Avant, uid réelle : %d , uid effective : %d\n", real, effective);
				seteuid(real);
				if (dflag)
					printf("Après, uid réelle : %d , uid effective : %d\n", (int)getuid(), (int)geteuid());
				p1 = fork();
				switch (p1)
				{
				case -1:
					fprintf(stderr, "erreur fork\n");
					exit(1);
					break;
				case 0: //On est dans le fils
					execlp("gzip", "gzip", "-n", tmp2, NULL);
					break;
				default: //On est dans le père
					if (wait(&status) == -1)
					{
						fprintf(log, "Le wait n'a pas fonctionné\n");
						perror("wait:");
						exit(EXIT_FAILURE);
					}
					free(tmp2);					
					seteuid(effective);
					//Si l'option d est activée, on écrit dans fichlog des infos
					//parfois utiles
					if (dflag)
						printf("\nJe suis après gzip !\n status : %d", status);

					if (dflag)
						printf("\nJe suis après fscanf !\n");

					mytime = time(NULL);
					char *timetmp = concat("", ctime(&mytime));
					//On enlève le '\n' ajouté par ctime
					timetmp[strlen(timetmp) - 1] = '\0';

					char *nomziptmp = concat(d->d_name, ".gz");
					char *nomzip = concat(chemin_spool, nomziptmp);

					if (dflag)
						printf("\nJe suis avant stat, nomziptmp : %s\n nomzip : %s\n",
						nomziptmp, nomzip);
					CHECK(stat(nomziptmp, stbuf) != MONEOF);

					if (dflag)
						printf("\nJe suis après stat !\n");

					fprintf(log, "id=%s orgdate=%s %s  %d %d:%d:%d %d user=%s ",
							&d->d_name[strlen(d->d_name) - 6],
							jour, mois, numjour, heure, minute, seconde, annee,
							login);
					fprintf(log, "file=%s orgsize=%d\n\tcurdate=%s gzipsize=%d exit=%d\n",
							nomfich, taille, timetmp, (int)stbuf->st_size, status);

					if (dflag)
						printf("\nJe suis après rename !\n");

					free(nomzip);
					free(nomziptmp);
					free(timetmp);
					unlink(chemin_d);
					unlink(chemin_j);
					fclose(dfich);
				}
				free(tmp);
				free(stbuf);
				free(chemin_d);
				free(chemin_j);
			}
			fclose(log);
		}
		free(chemin_spool);
		closedir(dp);
		//On délocke dans le cas où le spool est vide
		lockf(lfp, F_ULOCK, 0);
		close(lfp);
		//Si l'option i a été activée, on relance dans "delai" secondes
		if (iflag)
			sleep(delai);
		//10 secondes sinon
		else
			sleep(10);
	}
}

int main(int argc, char **argv)
{
	int c, dflag = 0, fflag = 0, iflag = 0;
	extern char *optarg;
	int input;
	while ((c = getopt(argc, argv, "dfi:")) != MONEOF)
	{
		switch (c)
		{
		case 'd':
			dflag++;
			break;

		case 'f':
			fflag++;
			break;

		case 'i':
			input = atoi(optarg);
			iflag++;
			break;

		default:
			print_usage();
			break;
		}
	}

	if (argc > 6 || argc < 2)
		print_usage();

	if (fflag == 0)
		init_daemon();
	demon(dflag, iflag, input, argv[argc - 1]);
}
