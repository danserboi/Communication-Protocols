#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}
// se extrag parametri dintr-un sir
void extract_parameters(char operation[1000], char parameters[10][100], int *n) {
	char parameter[50];
	int k;
	unsigned int i;
	k = -1, *n = -1;
	for (i = 0; i <= 2; i++)
		strcpy(parameters[i], "");
	for (i = 0; i <= strlen(operation); i++)
		if (operation[i] != ' ' && operation[i] != '\0' 
			&& operation[i] != '\n') {
			k++;
			parameter[k] = operation[i];
		} else {
			parameter[k + 1] = '\0';
			if (strlen(parameter) > 0)
			{
				(*n)++;
				strcpy(parameters[*n], parameter);
			}
			k = -1;
			strcpy(parameter, "");
		}
	(*n)++;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	// se adauga file descriptor pentru citirea de la tastatura
	FD_SET(0, &read_fds);

	int nr_clienti = 0;
	list resurse = NULL;

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if(i == 0) {
					// se citeste de la tastatura
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					if (strcmp(buffer, "quit\n") == 0) {
						// inchidere server
						goto closeserver;
					} 
					if(strcmp(buffer, "status\n") == 0) {
						printf("Numarul de clienti este: %d.\n\n", nr_clienti);
						printf("Acestea sunt paginile cu continutul lor:\n\n");
						list p = resurse;
						while(p) {
							resursa * res = p->element;
							printf("Pagina %s are continutul: %s.\n", res->nume_folder, res->continut);
							p = p->next;
						}
						fflush(stdout);
					}
				}
				else if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");
					
					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
					
					printf("Noua conexiune de la %s, port %d, socket client %d\n",
							inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
					

					// creste nr de clienti
					nr_clienti++;

				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {
						// conexiunea s-a inchis
						printf("Socket-ul client %d a inchis conexiunea\n", i);
						close(i);
						
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);

						// scoatem clientul din lista
						nr_clienti--;
					} else {
						printf ("S-a primit de la clientul de pe socketul %d mesajul: %s\n", i, buffer);
						char response[BUFLEN];
						memset(response, 0 ,BUFLEN);
						int no_par;
						char par[10][100];
						extract_parameters(buffer, par, & no_par);
						if(strcmp(par[0], "GET") == 0) {
							list celula_pagina = search(par[1], resurse, strlen(par[1]));
							if(celula_pagina) {
								resursa* res = celula_pagina->element;
								strcat(response, "HTTP/1.1 200 OK\r\n\r\n");
								strcat(response, res->continut);
								int s = send(i, response, strlen(response), 0);
								DIE(s < 0, "Failed to send to client.\n");
							} else {
								strcat(response, "HTTP/1.1 404 Not Found\r\n\r\n");
								int s = send(i, response, strlen(response), 0);
								DIE(s < 0, "Failed to send to client.\n");
							}
						}
						if(strcmp(par[0], "PUT") == 0) {
							list celula_pagina = search(par[1], resurse, strlen(par[1]));
							if(celula_pagina) {
								strcpy(celula_pagina->element, par[4]);
							} else {
								resursa* res = calloc(1, sizeof(resursa));
								strcpy(res->nume_folder, par[1]);
								strcpy(res->continut, par[4]);
								int c = cons(res, &resurse);
							}
							strcat(response, "HTTP/1.1 200 OK\r\n\r\n");
							int s = send(i, response, strlen(response), 0);
							DIE(s < 0, "Failed to send to client.\n");
						}
						if(strcmp(par[0], "DELETE") == 0) {
							list celula_pagina = search(par[1], resurse, strlen(par[1]));
							if(celula_pagina) {
								resursa* res = celula_pagina->element;
								int e = elim(celula_pagina->element, &resurse, strlen(celula_pagina->element));
								strcat(response, "HTTP/1.1 200 OK\r\n\r\n");
								int s = send(i, response, strlen(response), 0);
								DIE(s < 0, "Failed to send to client.\n");
							} else {
								strcat(response, "HTTP/1.1 404 Not Found\r\n\r\n");
								int s = send(i, response, strlen(response), 0);
								DIE(s < 0, "Failed to send to client.\n");
							}
						}
						fflush(stdout);
					}
				}
			}
		}
	}
closeserver: ;
	close(sockfd);
	free_all(resurse);
	return 0;
}
