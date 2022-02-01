/*=============================================*/
/* processus de creation d'une zone de memoire */
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
#define AREA_NAME       "PHRASE2"    /* ->nom de la zone partagee                 */
#define STOP            "STOP"      /* ->chaine a saisir pour declencher l'arret */
#define STR_LEN         256         /* ->taille par defaut des chaines           */
/*######*/
/* main */
/*######*/
int main( void )
{
    void *vAddr;                    /* ->adresse virtuelle sur la zone          */
    char *szInStr;                  /* ->chaine saisie                          */
    int  iShmFd;                    /* ->descripteur associe a la zone partagee */
    /*..................................*/
    /* tentative de creation de la zone */
    /*..................................*/
    if( (iShmFd = shm_open(AREA_NAME, O_RDWR | O_CREAT, 0600)) < 0)
    {
        /* on essaie de se lier sans creer... */
        printf("echec de creation, lien seul...\n");
        if( (iShmFd = shm_open(AREA_NAME, O_RDWR, 0600)) < 0)
        {  
            fprintf(stderr,"ERREUR : ---> appel a shm_open()\n");
            fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
            return( -errno );
        };
    };
    /* on attribue la taille a la zone partagee */
    ftruncate(iShmFd, STR_LEN);
    /* tentative de mapping de la zone dans l'espace memoire du */
    /* processus                                                */
    if( (vAddr = mmap(NULL, STR_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, iShmFd, 0 ))  == NULL)
    {
        fprintf(stderr,"ERREUR : ---> appel a mmap()\n");
        fprintf(stderr,"         code  = %d (%s)\n", errno, (char *)(strerror(errno)));
        return( -errno );
    };
    szInStr = (char *)(vAddr);
    /* saisie */
    do
    {
        fflush( stdin );
        scanf("%s",szInStr);
        if( strcmp(szInStr,STOP) == 0 )
        {
            break;
        };
    }
    while( 1 );
    printf("FIN\n");
    shm_unlink(AREA_NAME);
    return( 0 );
}