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
#define AREA_STATE_L       "STATE_L"      /* ->nom de la zone d'ecriture state_l                 */
#define AREA_STATE_R       "STATE_R"      /* ->nom de la zone d'ecriture state_l                 */

#define STOP            "STOP"      /* ->chaine a saisir pour declencher l'arret */
#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

#define NBR_ARG 1

/*######*/
/* main */
/*######*/
int main(int argc, char *argv[])
{
    double *u;                    /* ->variable partagee pour commande                */
    double *w_k;                  /* ->variable partagee pour la vitesse de rotation  */
    double *i_k;                  /* ->variable partagee pour l'intensite du moteur   */
    char areaState[STR_LEN];

    char areaCommande[STR_LEN];


    /* verification qu'il y a le bon nombre d'argument */
    if (argc != NBR_ARG + 1) {
        fprintf(stderr,"ERREUR : ---> nombre d'arguments invalides\n");
        return 1;
    }

    if (*argv[1] == 'L')
    {
        strcpy(areaCommande, AREA_COMMANDE_L);
        strcpy(areaState, AREA_STATE_L);

    }
    else if (*argv[1] == 'R')
    {
        strcpy(areaCommande, AREA_COMMANDE_R);
        strcpy(areaState, AREA_STATE_R);
    } else
    {
        fprintf(stderr,"ERREUR : ---> parametre directionnel non reconnu\n");
        return 1;
    }

    void *vAddrCommande;                    /* ->adresse virtuelle sur la zone          */
    int  iShmFdCommande;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone */
    /*..................................*/
    /* on essaie de se lier sans creer... */
    if( (iShmFdCommande = shm_open(areaCommande, O_RDWR, 0600)) < 0)
    {  
        fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdCommande, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrCommande = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdCommande, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    void *vAddrState;                    /* ->adresse virtuelle sur la zone          */
    double *state[2];                  /* ->variable partagee pour State                        */
    int  iShmFdState;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone */
    /*..................................*/
    /* on essaie de se lier sans creer... */
    if( (iShmFdState = shm_open(areaState, O_RDWR, 0600)) < 0)
    {  
        fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdState, 2*MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrState = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdState, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    u  = (double *)(vAddrCommande);
    w_k = (double *)(vAddrState);
    i_k = (double *)(vAddrState + sizeof(double));

    *u = 0.0;
    *w_k = 0.0;
    *i_k = 0.0;
    printf("w = %lf\t i = %lf\n", *w_k, *i_k);
    printf("FIN\n");
    //shm_unlink(areaState);
    return( 0 );
}