/*=============================================*/
/* processus de lecture d'une zone de memoire  */
/* partagee avec les appels POSIX              */
/*=============================================*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
/*............*/
/* constantes */
/*............*/
#define AREA_COMMANDE_L    "COMMAND_L"    /* ->nom de la zone de lecture commande_l              */
#define AREA_COMMANDE_R    "COMMAND_R"    /* ->nom de la zone de lecture commande_r              */

#define STOP            "STOP"      /* ->chaine a saisir pour declencher l'arret */
#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

/*######*/
/* main */
/*######*/
int main(int argc, char *argv[])
{
    int is_right;
    char areaCommande[STR_LEN];


    /* verification qu'il y a le bon nombre d'argument */
    if (argc != 2) {
        fprintf(stderr,"ERREUR : ---> nombre d'arguments invalides\n");
        return 1;
    }

    if (*argv[1] == 'L')
    {
        is_right = 0;
        strcpy(areaCommande, AREA_COMMANDE_L);

    }
    else if (*argv[1] == 'R')
    {
        is_right = 1;
        strcpy(areaCommande, AREA_COMMANDE_R);
    } else
    {
        fprintf(stderr,"ERREUR : ---> parametre directionnel non reconnu\n");
        return 1;
    }

    void *vAddr;                    /* ->adresse virtuelle sur la zone          */
    double *u;                  /* ->variable partagee pour commande                        */
    int  iShmFd;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone */
    /*..................................*/
    /* on essaie de se lier sans creer... */
    if( (iShmFd = shm_open(areaCommande, O_RDWR, 0600)) < 0)
    {  
        fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFd, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddr = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFd, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    u = (double *)(vAddr);

    /* saisie */
    do
    {
        fflush( stdin );
        scanf("%lf", u);
    }
    while( 1 );
    printf("FIN\n");
    shm_unlink(areaCommande);
    return( 0 );
}