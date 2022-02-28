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
#define AREA_COMMANDE_R    "COMMAND_R"    /* ->nom de la zone de lecture commande_r              */
#define AREA_STATE_R       "STATE_R"      /* ->nom de la zone d'ecriture state_l                 */

#define STR_LEN         256         /* ->taille par defaut des chaines           */
#define MEMORY_LEN      64

#define NBR_ARG 8

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
    double *u;                    /* ->variable partagee pour commande                */
    double *w_k;                  /* ->variable partagee pour la vitesse de rotation  */
    double *i_k;                  /* ->variable partagee pour l'intensite du moteur   */
    double w_k1;                  /* ->variable pour la vitesse de rotation au temps k+1 */
    double i_k1;                  /* ->variablepour l'intensiter du moteur au temps k+1  */


    double R; /* resistance */
    double L; /* inductance */
    double Ke; /* constante electrique */
    double Km; /* constante moteur */
    double f; /* coefficient de frottement */
    double J; /* inercite sur le rotor */
    double Te; /* periode d'echantillonage */

    double z0;
    double z1;
    double b0;
    double b1;


    char areaCommande[STR_LEN];
    char areaState[STR_LEN];


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
        fprintf(stderr,"ERREUR : ---> parametre 1: Directionnel non reconnu\n");
        return 1;
    }

    if (sscanf(argv[2], "%lf", &R)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 2: Resistance doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[3], "%lf", &L)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 3: Inductance doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[4], "%lf", &Ke)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 4: Constante electrique doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[5], "%lf", &Km)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 5: Constante moteur doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[6], "%lf", &f)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 6: Coefficient de frottement doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[7], "%lf", &J)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 7: Inercie doit etre un double\n");
        return 1;
    }
    if (sscanf(argv[8], "%lf", &Te)  == 0) 
    {
        fprintf(stderr,"ERREUR : ---> parametre 8: Temps d'echantillon doit etre un double\n");
        return 1;
    }

    /* calcul des constantes z et b*/
    z0 = exp(-Te * R / L);
    b0 = (1 - z0) / R;
    z1 = exp(-Te * f / J);
    b1 = Km * (1 - z1) / f;

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
    if( (vAddrState = mmap(NULL, 2*MEMORY_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFdState, 0 ))  == NULL)
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




    u = (double *)(vAddrCommande);
    w_k = (double *)(vAddrState);
    i_k = (double *)(vAddrState + sizeof(double));

    *i_k = b0 * (*u);
    *w_k = b1 * (*i_k);
    /* affichage + calcul */
    do
    {
        i_k1 = z0 * (*i_k) - Ke * b0 * (*w_k) + b0 * (*u);
        w_k1 = z1 * (*w_k) + b1 * (*i_k);

        *i_k = i_k1;
        *w_k = w_k1;
        printf("u = %lf\t w = %lf\t i = %lf\n", *u, *w_k, *i_k);
        sleep(1);

    }
    while( g_run );
    //shm_unlink(AREA_NAME);
    return( 0 );
}
