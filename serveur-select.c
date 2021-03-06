/* echo / serveur simpliste
   Master Informatique 2012 -- Université Aix-Marseille  
   Emmanuel Godard
*/
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>

/* taille maximale des lignes */
#define MAXLIGNE 80
#define CIAO "Au revoir ...\n"

char echo(int f, char* hote, char* port, char politesse);

int main(int argc, char *argv[])
{
  int s,n; /* descripteurs de socket */
  int len,on; /* utilitaires divers */
  struct addrinfo * resol; /* résolution */
  struct addrinfo indic = {AI_PASSIVE, /* Toute interface */
                           PF_INET,SOCK_STREAM,0, /* IP mode connecté */
                           0,NULL,NULL,NULL};
  char * port; /* Port pour le service */
  int err; /* code d'erreur */
  
  /* Traitement des arguments */
  if (argc!=2) { /* erreur de syntaxe */
    printf("Usage: %s port\n",argv[0]);
    exit(1);
  }
  
  port=argv[1]; fprintf(stderr,"Ecoute sur le port %s\n",port);
  err = getaddrinfo(NULL,port,&indic,&resol); 
  if (err<0){
    fprintf(stderr,"Résolution: %s\n",gai_strerror(err));
    exit(2);
  }

  /* Création de la socket, de type TCP / IP */
  if ((s=socket(resol->ai_family,resol->ai_socktype,resol->ai_protocol))<0) {
    perror("allocation de socket");
    exit(3);
  }
  fprintf(stderr,"le n° de la socket est : %i\n",s);

  /* On rend le port réutilisable rapidement /!\ */
  on = 1;
  if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0) {
    perror("option socket");
    exit(4);
  }
  fprintf(stderr,"Option(s) OK!\n");

  /* Association de la socket s à l'adresse obtenue par résolution */
  if (bind(s,resol->ai_addr,sizeof(struct sockaddr_in))<0) {
    perror("bind");
    exit(5);
  }
  freeaddrinfo(resol); /* /!\ Libération mémoire */
  fprintf(stderr,"bind!\n");

  /* la socket est prête à recevoir */
  if (listen(s,SOMAXCONN)<0) {
    perror("listen");
    exit(6);
  }
  fprintf(stderr,"listen!\n");
  fd_set rdfs;
  int actual = 0;
  int max = s;
  int clients[42];
  while(1) 
  {
	int i = 0;
	int j = 0;
    	FD_ZERO(&rdfs);
	FD_SET(s, &rdfs);
	for( i = 0; i < actual; i++)
	{
		FD_SET(clients[i], &rdfs);
	}
	if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
	{
		printf("error\n");
		exit(2);
	}
	struct sockaddr_in csin;
	len=sizeof(struct sockaddr_in);
	char hotec[NI_MAXHOST];
	char portc[NI_MAXSERV];
	if(FD_ISSET(s, &rdfs))
	{
		printf("Un nouveau client\n");
		len = sizeof csin;
		if( (n = accept(s, (struct sockaddr *) &csin, (socklen_t*)&len)) < 0 )
		{
      			perror("accept");
     			exit(7);
    		}
		err = getnameinfo((struct sockaddr*)&csin,len,hotec,NI_MAXHOST,portc,NI_MAXSERV,0);
		if (err < 0 )
		{
      			fprintf(stderr,"résolution client (%i): %s\n",n,gai_strerror(err));
    		}
		else
		{ 
      			fprintf(stderr,"accept! (%i) ip=%s port=%s\n",n,hotec,portc);
    		}
	  	echo(n,hotec,portc, 1);
		max = n > max ? n : max;
		FD_SET(n, &rdfs);
		clients[actual++] = n;
  	}
  	else
  	{
         	for(j = 0; j < actual; j++)
         	{
           		 /* a client is talking */
            		if(FD_ISSET(clients[j], &rdfs))
            		{
				err = getnameinfo((struct sockaddr*)&clients[j],len,hotec,NI_MAXHOST,portc,NI_MAXSERV,0);
	  			if ( echo(clients[j],hotec,portc, 0) == 0)
				{
					close(clients[j]);
					//remove client socket
					memmove(clients + j, clients + j + 1, (actual - j - 1) * sizeof(int));
					actual--;
				}	
			}
		}
  	}	
  }
  return EXIT_SUCCESS;
}

/* echo des messages reçus (le tout via le descripteur f) */
char echo(int f, char* hote, char* port, char politesse)
{
  ssize_t lu; /* nb d'octets reçus */
  char msg[MAXLIGNE+1]; /* tampons pour les communications */ 
  char tampon[MAXLIGNE+1]; 
  int pid = getpid(); /* pid du processus */
  
  /* message d'accueil */
  if(politesse)
  {
  	snprintf(msg,MAXLIGNE,"Bonjour %s! (vous utilisez le port %s)\n",hote,port);
  	/* envoi du message d'accueil */
  	send(f,msg,strlen(msg),0);
  }
  else
  {
  	/* Faire echo et logguer */
  	lu = recv(f,tampon,MAXLIGNE,0);
	
  	if (lu <= 0 )
  	{
		return 0;
  	}
  	tampon[lu] = '\0';  
  	fprintf(stderr,"[%s:%s](%i) :%s\n",hote,port,pid,tampon);
  	snprintf(msg,MAXLIGNE,"> %s",tampon);
        /* echo vers le client */
  	send(f, msg, strlen(msg),0);
  }  
  return 1;
}
