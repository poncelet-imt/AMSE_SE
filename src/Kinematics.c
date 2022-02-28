/*=============================================*/
/* processus de lecture d'une zone de memoire  */
/* partagee avec les appels POSIX              */
/*=============================================*/
#include <signal.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
/*............*/
/* constantes */
/*............*/
#define AREA_STATE_L       "STATE_L"      /* ->nom de la zone d'ecriture state_l                 */
#define AREA_STATE_R       "STATE_R"      /* ->nom de la zone d'ecriture state_l                 */

#define AREA_VELOCITY    "VELOCITY"    /* ->nom de la zone de lecture commande_l              */

#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

#define NBR_ARG 3

int g_run= 1;


/*######*/
/* main */
/*######*/
int main(int argc, char *argv[])
{

    double *w_l;                  /* ->variable partagee pour la vitesse de rotation gauche */
    double *w_r;                  /* ->variable partagee pour la vitesse de rotation droite */
    double *w_c;                  /* ->variable partagee pour la vitesse de rotation centrale */
    double *v_c;                  /* ->variable partagee pour la vitesse centrale du robot */

    //double *i_l;                  /* ->variable partagee pour l'intensite du moteur gauche */
    //double *i_r;                  /* ->variable partagee pour l'intensite du moteur droit */

    double W; /* entraxe */
    double R0; /* Rayon commun des roues */
    double Te; /* periode d'échantillonage */


    char areaStateLeft[STR_LEN];
    char areaStateRight[STR_LEN];
    char areaVelocity[STR_LEN];

    /* verification qu'il y a le bon nombre d'argument */
    if (argc != NBR_ARG + 1) {
        fprintf(stderr,"ERREUR : ---> nombre d'arguments invalides\n");
        return 1;
    }

    strcpy(areaStateLeft, AREA_STATE_L);
    strcpy(areaStateRight, AREA_STATE_R);
    strcpy(areaVelocity, AREA_VELOCITY);


    if (sscanf(argv[1], "%lf", &W)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 1: Entraxe doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[2], "%lf", &R0)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 2: Rayon commun des roues doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[3], "%lf", &Te)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 5: Periode échantillonage moteur doit etre un double\n");
        return 1;
    }


    void *vAddrStateLeft; /* ->adresse virtuelle sur la zone          */
    int  iShmFdStateLeft;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone commande */
    /*..................................*/
     if( (iShmFdStateLeft = shm_open(areaStateLeft, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdStateLeft = shm_open(areaStateLeft, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdStateLeft, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrStateLeft = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdStateLeft, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    
    void *vAddrStateRight; /* ->adresse virtuelle sur la zone          */
    int  iShmFdStateRight;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone commande */
    /*..................................*/
     if( (iShmFdStateRight = shm_open(areaStateRight, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdStateRight = shm_open(areaStateRight, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdStateRight, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrStateRight = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdStateRight, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };


    void *vAddrVelocity; /* ->adresse virtuelle sur la zone          */
    int  iShmFdVelocity;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone commande */
    /*..................................*/
     if( (iShmFdVelocity = shm_open(areaVelocity, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdVelocity = shm_open(areaVelocity, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdVelocity, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrVelocity = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdVelocity, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    //CAST ZONES PARTAGEES
    w_l = (double *)(vAddrStateLeft);
    w_r = (double *)(vAddrStateRight);
    w_c = (double *)(vAddrVelocity);
    v_c = (double *)(vAddrVelocity + sizeof(double));
    
    /* affichage + calcul */
    do
    {
        *v_c = R0 * ( *(w_l) + *(w_r)) / 2;
        *w_c = R0 * ( *(w_r) - *(w_l) ) / W;

        printf("w_l = %lf\t w_r = %lf\t w_c = %lf\t v_c = %lf\n", *w_l, *w_r, *w_c, *v_c);
        sleep(1);
    }
    while( g_run );
    //shm_unlink(AREA_NAME);
    return( 0 );
}
