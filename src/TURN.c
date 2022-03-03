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


#define AREA_POSITION    "POSITION"    /* ->nom de la zone de lecture position              */
#define AREA_TARGET_L    "TARGET_L"    /* ->nom de la zone de lecture target_l              */
#define AREA_TARGET_R    "TARGET_R"    /* ->nom de la zone de lecture target_r              */

#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

#define EPSILON 0.0005

#define NBR_ARG 2

int g_run= 1;

/*............*/
/* prototype  */
/*............*/
extern char *strsignal( int);
void signal_handler( int ); /* ->routine de gestion du signal recu */
void signal_handler( int signal ) /* ->code du signal recu */
{
    g_run = 0;
    /* strsignal retourne la chaine de caracteres */
    /* qui correspond au "nom symbolique" du signal */
    printf("%s\n", (char *)(strsignal( signal )) );
}


/*######*/
/* main */
/*######*/
int main(int argc, char *argv[])
{
    double *tvR;                  /* ->variable partagee pour target right                       */
    double *tvL;                  /* ->variable partagee pour target left                       */

    double *x;                  /* ->variable partagee pour la position x du robot */
    double *y;                  /* ->variable partagee pour la position y du robot */
    double *q;                  /* ->variable partagee pour l'orientation du robot */

    double x_0;                  /* ->variable pour la position x du robot (à k = 0) */
    double y_0;                  /* ->variable pour la position y du robot (à k = 0) */
    double q_0;                  /* ->variable pour l'orientation du robot (à k = 0) */
    double errorAngle;


    double angle; /* Angle a parcourir */
    double w; /* Vitesse de rotation des moteurs */


    char areaTargetLeft[STR_LEN];
    char areaTargetRight[STR_LEN];
    char areaPosition[STR_LEN];

    /* verification qu'il y a le bon nombre d'argument */
    if (argc != NBR_ARG + 1) {
        fprintf(stderr,"ERREUR : ---> nombre d'arguments invalides\n");
        return 1;
    }

    strcpy(areaTargetLeft, AREA_TARGET_L);
    strcpy(areaTargetRight, AREA_TARGET_R);
    strcpy(areaPosition, AREA_POSITION);



    if (sscanf(argv[1], "%lf", &angle)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 1: angle a parcourir doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[2], "%lf", &w)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 1: Vitesse de rotation des moteurs doit etre un double\n");
        return 1;
    }

    void *vAddrTargetLeft;                    /* ->adresse virtuelle sur la zone          */
    int  iShmFdTargetLeft;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone */
    /*..................................*/
    /* on essaie de se lier sans creer... */
    if( (iShmFdTargetLeft = shm_open(areaTargetLeft, O_RDWR, 0600)) < 0)
    {  
        fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdTargetLeft, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrTargetLeft = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdTargetLeft, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    void *vAddrTargetRight;                    /* ->adresse virtuelle sur la zone          */
    int  iShmFdTargetRight;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone */
    /*..................................*/
    /* on essaie de se lier sans creer... */
    if( (iShmFdTargetRight = shm_open(areaTargetRight, O_RDWR, 0600)) < 0)
    {  
        fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdTargetRight, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrTargetRight = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdTargetRight, 0 ))  == NULL)
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
    ftruncate(iShmFdPosition, 3*MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrPosition = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdPosition, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    struct sigaction sa; /* ->structure permettant de definir le gestionnaire */
    /* et d'y associer le signal a traiter, etc. */
    sigset_t blocked; /* ->contiendra la liste des signaux qui seront masques */
    /* on ne bloque aucun signal : blocked = vide */
    sigemptyset( &blocked );
    /* mise a jour de la structure sigaction */
    memset( &sa, 0, sizeof(struct sigaction));
    sa.sa_handler = signal_handler; /* ->precise le gestionnaire a utiliser */
    sa.sa_flags = 0; /* ->fonctionnement "normal" */
    sa.sa_mask = blocked; /* ->indique les signaux masques */
    /* installation EFFECTIVE du gestionnaire */
    sigaction( SIGUSR1, &sa, NULL ); /* ->associe signal_handler a la reception de SIGUSR1 */

    
    //vAddrPosition
    //CAST ZONES PARTAGEES

    tvR = (double *)(vAddrTargetLeft);
    tvL = (double *)(vAddrTargetRight);


    x = (double *)(vAddrPosition);
    y = (double *)(vAddrPosition + sizeof(double));
    q = (double *)(vAddrPosition + 2 * sizeof(double));

    //INIT k=0

    x_0 = *x;
    y_0 = *y;
    q_0 = *q;
    
    errorAngle =  fabs(*q - q_0) - fmod(angle, 2*M_PI);
    /* affichage + calcul */
    do
    {
        
        errorAngle = fabs(*q - q_0) - fmod(angle, 2*M_PI);
        if (errorAngle > 0.0)
        {
            *tvR = -w;
            *tvL = w;
        } else if (errorAngle < 0.0)
        {
            *tvR = w;
            *tvL = -w;
        } else {
            *tvR = 0.0;
            *tvL = 0.0;
        }
        usleep(1000);
    }
    while( g_run && (fabs(errorAngle) > EPSILON) );
    //shm_unlink(AREA_NAME);
    *tvR = 0.0;
    *tvL = 0.0;
    return( 0 );
}
