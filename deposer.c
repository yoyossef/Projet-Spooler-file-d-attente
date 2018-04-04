/**
 * \file deposer.c
 * \brief Programme qui dépose un job dans le spool
 * \author ROSTAQI Yossef et PENG Zhigang
 * \date 7 Novembre 2017
 *
 *
 */

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

char *concat(char *a, char *b)
{
    char *dest = (char *)malloc(strlen(a) + strlen(b) + 1);
    strcpy(dest, a);
    strcat(dest, b);
    return dest;
}


/**
 * \fn void deposer(char ** fichiers, int nb)
 * \brief Dépose un fichier dans le spool, sous un nom généré aléatoirement
 * et avec un préfixe j pour le job et d pour ses metadonnées
 *
 * \param char ** fichiers
 * \param int nb, le nombre de fichiers à déposer
 * \return void
 */

void deposer(char ** fichiers, int nb){
    int lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
    while (lockf(lfp, F_TEST, 0) != 0)
    {
        sleep(1); /* can not lock */
    }
    lockf(lfp, F_LOCK, 0);
    pid_t p1, p2;
    int statut;
    FILE * fd;
    char * chemin_spool = NULL;
    if(getenv("PROJETSE") != NULL){
        chemin_spool = concat(getenv("PROJETSE"), "/");
    }
    else{
        chemin_spool = concat(SPOOL, "/");
    }
    time_t mytime;
    int i = 1;
    for(i = 1; i<nb; i++){
        char * nom_j = concat("j","XXXXXX");
        char * chemin_j = concat(chemin_spool, nom_j);
        struct stat * stbuf = malloc(sizeof(struct stat));
        CHECK(stbuf != NULL);
        mkstemp(chemin_j);
        char * nom_d = concat("d", &chemin_j[strlen (chemin_j) - 6]);
        char * chemin_d = concat(chemin_spool, nom_d);
        //On sauvegarde l'uid réel
        uid_t real = getuid();
        //On sauvegarde l'uid effectif
        uid_t effective = geteuid();
        //user effectif = user réel, pour éviter les soucis de droits
        //potentiellement protégés par son propriétaire (l'utilisateur réel)
        seteuid(real);
        p1 = fork();
        switch(p1){
            case -1 :
                fprintf(stderr,"erreur fork\n");
                exit(1);
                break;
            case 0 :
                //On copie dans un premier temps le fichier à déposer dans le
                //répertoire courant, puis on le déplacera avec les droits du
                //propriétaire de l'exécutable pour veiller à la sécurité du spool
                execlp("cp", "cp", fichiers[i], &chemin_j[strlen (chemin_j) - 7], NULL);
                break;
            default :
                p2 = wait(&statut);
                if  (p2 == -1){
                    fprintf(stderr,"erreur wait");
                }
                //On repasse aux droits du propriétaire de l'executable pour
                //déplacer le fichier vers le spool ou gérer le cas d'erreur
                seteuid(effective);
                if(statut != 0){
                    fprintf(stderr, "Le cp a échoué\n");
                    //abort: on supprime le fichier créé par mkstemp 
                    //et on quitte avec le code d'erreur 1
                    unlink(chemin_j);
                    exit(1);
                }
                //on déplace le fichier dans le spool
                rename(&chemin_j[strlen (chemin_j) - 7], chemin_j);
                //on ouvre en écriture le fichier "d"
                fd = fopen(chemin_d, "w");
                CHECK(fd != NULL);
                CHECK(stat(fichiers[i], stbuf) != MONEOF);
                mytime = time(NULL);
                fprintf(fd, "%s %d %s %s %d\n", getlogin(), (int)stbuf -> st_size, fichiers[i], ctime(&mytime), (int) stbuf -> st_uid);
                fclose(fd);
                printf("%s \n", &chemin_j[strlen (chemin_j) - 6]);
                free(nom_j);
                free(chemin_j);
                free(nom_d);
                free(chemin_d);
                free(stbuf);
        }
    }
    free(chemin_spool);
    //on délocke le fichier verrou pour indiquer que le spool est libre
    lockf(lfp, F_ULOCK, 0);
    close(lfp);
}

int main(int argc, char *argv[])
{
    if (argc < (1 + 1))
    {
        fprintf(stderr, "déposer : Usage incorrect, besoin d'au moins un argument \n Usage : deposer fichier");
        exit(1);
    }
    if (getenv("PROJETSE") == NULL && access(SPOOL, F_OK) == MONEOF){
	fprintf(stderr, "déposer : le spool indiqué n'existe pas");
	exit(1);
    }
    deposer(argv, argc);
    return 0;
}
