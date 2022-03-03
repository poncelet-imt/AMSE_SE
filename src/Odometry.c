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
    double t;
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
    ftruncate(iShmFdVelocity, 2*MEMORY_LEN);
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
    w_c = (double *)(vAddrVelocity);
    v_c = (double *)(vAddrVelocity + sizeof(double));

    x_k = (double *)(vAddrPosition);
    y_k = (double *)(vAddrPosition + sizeof(double));
    q_k = (double *)(vAddrPosition + 2 * sizeof(double));

    //INIT k=0
    *x_k = 0.0;
    *y_k = 0.0;
    *q_k = 0.0;
    t = 0.0;
    
    /* affichage + calcul */
    printf("t,w_c,v_c,x,y,q\n");
    do
    {
        t += Te;
        x_k1 = (*x_k) - (*v_c) * Te * sin((*q_k));
        y_k1 = (*y_k) + (*v_c) * Te * cos((*q_k));
        q_k1 = (*q_k) + Te * (*w_c);
        
        *x_k = x_k1;
        *y_k = y_k1;
        *q_k = q_k1;
        printf("%lf,%lf,%lf,%lf,%lf,%lf,\n", t, *w_c, *v_c, *x_k, *y_k, *q_k);
        fflush(stdout);
        sleep(1);
    }
    while( g_run );
    //shm_unlink(AREA_NAME);
    return( 0 );
}
