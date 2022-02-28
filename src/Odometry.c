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


#define AREA_VELOCITY    "VELOCITY"    /* ->nom de la zone de lecture velocity           */
#define AREA_POSITION    "POSITION"    /* ->nom de la zone de lecture position              */

#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

#define NBR_ARG 1

int g_run= 1;


/*######*/
/* main */
/*######*/
int main(int argc, char *argv[])
{

    double *w_c;                  /* ->variable partagee pour la vitesse de rotation centrale */
    double *v_c;                  /* ->variable partagee pour la vitesse centrale du robot */

    double *x_k;                  /* ->variable partagee pour la position x du robot */
    double *y_k;                  /* ->variable partagee pour la position y du robot */
    double *q_k;                  /* ->variable partagee pour l'orientation du robot */

    double x_k1;                  /* ->variable partagee pour la position x du robot (à k + 1) */
    double y_k1;                  /* ->variable partagee pour la position y du robot (à k + 1) */
    double q_k1;                  /* ->variable partagee pour l'orientation du robot (à k + 1) */


    double Te; /* periode d'échantillonage */


    char areaVelocity[STR_LEN];
    char areaPosition[STR_LEN];

    /* verification qu'il y a le bon nombre d'argument */
    if (argc != NBR_ARG + 1) {
        fprintf(stderr,"ERREUR : ---> nombre d'arguments invalides\n");
        return 1;
    }

    strcpy(areaVelocity, AREA_VELOCITY);
    strcpy(areaPosition, AREA_POSITION);



    if (sscanf(argv[1], "%lf", &Te)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 1: Periode échantillonage moteur doit etre un double\n");
        return 1;
    }

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

    void *vAddrPosition; /* ->adresse virtuelle sur la zone          */
    int  iShmFdPosition;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone commande */
    /*..................................*/
     if( (iShmFdPosition = shm_open(areaPosition, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdPosition = shm_open(areaPosition, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdPosition, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrPosition = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdPosition, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    
    //vAddrPosition
    //CAST ZONES PARTAGEES
    w_c = (double *)(vAddrVelocity);
    v_c = (double *)(vAddrVelocity + sizeof(double));

    x_k = (double *)(vAddrVelocity);
    y_k = (double *)(vAddrVelocity + sizeof(double));
    q_k = (double *)(vAddrVelocity + sizeof(double) + sizeof(double));

    //INIT k=0
    x_k = 0;
    y_k = 0;
    q_k = 0;


    
    /* affichage + calcul */
    do
    {
        x_k1 = *(x_k) - *(v_c) * Te * math.sin(*(q_k));
        y_k1 = *(y_k) + *(v_c) * Te * math.cos(*(q_k));
        q_k1 = *(q_k) + Te * *(w_c);

        *x_k = x_k1;
        *y_k = y_k1;
        *q_k1 = q_k1;

        printf("w_c = %lf\t v_c = %lf\t x = %lf\t y = %lf\t q = %lf\n", *w_c, *v_c, *x_k, *y_k, *q_k);
    }
    while( g_run );
    //shm_unlink(AREA_NAME);
    return( 0 );
}
