/**
 * \file retirer.c
 * \brief Programme qui retirer un job du spool
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
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "check.h"

#define LOCK_FILE "verrou"
#define SPOOL "spool"
#define MONEOF -1

char *concat(char *a, char *b)
{
    char *dest = (char *)malloc(strlen(a) + strlen(b) + 1);
    strcpy(dest, a);
    strcat(dest, b);
    return dest;
}

/**
 * \fn void retirer(char **id, int nb)
 * \brief Retire les jobs correspondant aux ids dans le spool
 *
 * \param char **id
 * \param int nb, le nombre d'id entré par l'utilisateur
 * \return void
 */

void retirer(char **id, int nb)
{
    int lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
    CHECK(lfp != MONEOF);
    while (lockf(lfp, F_TEST, 0) != 0)
    {
        sleep(1); /* can not lock */
    }
    lockf(lfp, F_LOCK, 0);
    FILE *fd;
    struct stat *stbuf = malloc(sizeof(struct stat));
    if (getenv("PROJETSE") != NULL)
    {
        CHECK(stat(getenv("PROJETSE"), stbuf) != MONEOF);
    }
    else
    {
        CHECK(stat(SPOOL, stbuf) != MONEOF);
    }
    char *s = malloc(100); //sert à sauvegarder le login inscrit dans le dXXXXXX
    struct dirent *di;
    char *chemin = NULL;
    char *chemin2 = NULL;
    char *chemin_spool = NULL;
    if (getenv("PROJETSE") != NULL)
    {
        chemin_spool = concat(getenv("PROJETSE"), "/");
    }
    else
    {
        chemin_spool = concat(SPOOL, "/");
    }
    int trouve = 0;

    for (int i = 1; i < nb; i++)
    {
        DIR *dp = NULL;
        if (getenv("PROJETSE") != NULL)
        {
            dp = opendir(getenv("PROJETSE"));
        }
        else
        {
            dp = opendir(SPOOL);
        }
        CHECK(dp != NULL);

        if (strlen(id[i]) != 6)
        {
            fprintf(stderr, "Cannot find id %s\n", id[i]);
            exit(-1);
        }

        char *j = concat("j", id[i]);
        char *d = concat("d", id[i]);

        while ((di = readdir(dp)) != NULL)
        {
            if (strcmp(di->d_name, ".") != 0 && strcmp(di->d_name, "..") != 0 && di->d_name[0] != 'j')
            {
                chemin = concat(chemin_spool, di->d_name);
                chemin2 = concat(chemin_spool, j);
                if (strcmp(di->d_name, d) == 0)
                {
                    fd = fopen(chemin, "r");
                    CHECK(fd != NULL);
                    fscanf(fd, "%s", s); //on enregistre dans s la valeur du getlogin
                    char * login = NULL;
                    if(getlogin() == NULL)
                        login = "(null)";
                    else
                        login = getlogin();
                    //Si on a le droit de le retirer :
                    if (strcmp(s, login) == 0 || stbuf->st_uid == getuid()){
                        trouve++;
                        unlink(chemin);
                        unlink(chemin2);
                    }

                    free(chemin);
                    free(chemin2);
                    fclose(fd);
                }

                else{
                    free(chemin);
                    free(chemin2);
                }
            }
        }
        free(d);
        free(j);
        closedir(dp);

        if (trouve == 0){
            //Si l'utilisateur réel ne possède pas le spool
            if (stbuf->st_uid != getuid()){
                fprintf(stderr, "You don't own the spool or you didn't add the job yourself\n");
                free(s);
                free(chemin_spool);
                free(di);
                free(stbuf);
                lockf(lfp, F_ULOCK, 0);
                close(lfp);
                exit(1);
            }
            fprintf(stderr, "Cannot find id %s\n", id[i]);
            free(s);
            free(chemin_spool);
            free(di);
            free(stbuf);
            lockf(lfp, F_ULOCK, 0);
            close(lfp);
            exit(1);
        }
    }
    free(s);
    free(chemin_spool);
    free(di);
    free(stbuf);
    lockf(lfp, F_ULOCK, 0);
    close(lfp);
}

int main(int argc, char *argv[])
{
    if (argc < (1 + 1))
    {
        fprintf(stderr, "retirer : Usage incorrect, besoin d'au moins un argument \n Usage : retirer XXXXXX\n");
        exit(1);
    }
    retirer(argv, argc);
    return 0;
}