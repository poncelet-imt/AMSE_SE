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
#define AREA_COMMANDE_L    "COMMAND_L"    /* ->nom de la zone de lecture commande_l              */
#define AREA_STATE_L       "STATE_L"      /* ->nom de la zone d'ecriture state_l                 */
#define AREA_TARGET_L           "TARGET_L"     /* ->nom de la zone d'ecriture target_l                 */
#define AREA_COMMANDE_R    "COMMAND_R"    /* ->nom de la zone de lecture commande_r              */
#define AREA_STATE_R       "STATE_R"      /* ->nom de la zone d'ecriture state_l                 */
#define AREA_TARGET_R           "TARGET_R"     /* ->nom de la zone d'ecriture target_r                 */

#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

#define NBR_ARG 5

int g_run= 1;
int g_pause = 0;


/*............*/
/* prototype  */
/*............*/
extern char *strsignal( int);
void signal_handler( int ); /* ->routine de gestion du signal recu */
void signal_handler( int signal ) /* ->code du signal recu */
{
    if (signal == SIGUSR1) {
        g_run = 0;
    } else if (signal == SIGUSR2) {
        g_pause = 1 - g_pause;
        printf("Processus stoppe!");
    }
    /* qui correspond au "nom symbolique" du signal */
    printf("%s\n", (char *)(strsignal( signal )) );
}


/*######*/
/* main */
/*######*/
int main(int argc, char *argv[])
{
    double *u;                    /* ->variable partagee pour commande                */
    double *tv;                   /* ->variable partagee pour target                  */
    double *w_k;                  /* ->variable partagee pour la vitesse de rotation  */
    double *i_k;                  /* ->variable partagee pour l'intensite du moteur   */


    double K; /* action proportionnelle */
    double I; /* action intregrale */
    double D; /* action derivee */
    double T0; /* periode de l'alarme cyclique */

    double e_k; /* erreur courante */
    double E_k; /* Intergrale de l'erreur */
    double e_k1; /* e k-1 */
    double E_k1; /* E k-1 */


    char areaCommande[STR_LEN];
    char areaState[STR_LEN];
    char areaTarget[STR_LEN];


    /* verification qu'il y a le bon nombre d'argument */
    if (argc != NBR_ARG + 1) {
        fprintf(stderr,"ERREUR : ---> nombre d'arguments invalides\n");
        return 1;
    }

    if (*argv[1] == 'L')
    {
        strcpy(areaCommande, AREA_COMMANDE_L);
        strcpy(areaState, AREA_STATE_L);
        strcpy(areaTarget, AREA_TARGET_L);

    }
    else if (*argv[1] == 'R')
    {
        strcpy(areaCommande, AREA_COMMANDE_R);
        strcpy(areaState, AREA_STATE_R);
        strcpy(areaTarget, AREA_TARGET_R);
    } else
    {
        fprintf(stderr,"ERREUR : ---> parametre 1: Directionnel non reconnu\n");
        return 1;
    }

    if (sscanf(argv[2], "%lf", &K)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 2: Action proportionnelle doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[3], "%lf", &I)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 3: Action Intergrale doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[4], "%lf", &D)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 4: Action Derivee electrique doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[5], "%lf", &T0)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 5: Periode rafrechissement moteur doit etre un double\n");
        return 1;
    }

    void *vAddrCommande;                    /* ->adresse virtuelle sur la zone          */
    int  iShmFdCommande;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone commande */
    /*..................................*/
    if( (iShmFdCommande = shm_open(areaCommande, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdCommande = shm_open(areaCommande, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
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

    void *vAddrTarget;                    /* ->adresse virtuelle sur la zone          */
    int  iShmFdTarget;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone Target */
    /*..................................*/
    if( (iShmFdTarget = shm_open(areaTarget, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdTarget = shm_open(areaTarget, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFdTarget, MEMORY_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddrTarget = mmap(NULL, MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdTarget, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };

    void *vAddrState;                    /* ->adresse virtuelle sur la zone          */
    double *state[2];                  /* ->variable partagee pour State                        */
    int  iShmFdState;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative d'acces a la zone State */
    /*..................................*/
    if( (iShmFdState = shm_open(areaState, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFdState = shm_open(areaState, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
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
    sigaction( SIGUSR2, &sa, NULL ); /* ->associe signal_handler a la reception de SIGUSR2 */

    u = (double *)(vAddrCommande);
    tv = (double *)(vAddrTarget);
    w_k = (double *)(vAddrState);
    i_k = (double *)(vAddrState + sizeof(double));

    e_k = (*tv) - (*w_k);
    E_k = T0 * e_k;
    /* affichage + calcul */
    do
    {
        e_k1 = e_k;
        E_k1 = E_k;

        e_k = (*tv) - (*w_k);
        E_k = E_k1 + T0 * e_k; 

        *u = K * ( e_k + I * E_k + D * (e_k - e_k1) / T0 );
        printf("tv = %lf\t wk = %lf\t u = %lf\n", *tv, *w_k, *u);
        while (g_pause)
        {
            sleep(1);
        }
        
        sleep(1);

    }
    while( g_run );
    //shm_unlink(AREA_NAME);
    return( 0 );
}
