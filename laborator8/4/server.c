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
	// vector caracteristic pentru clienti(socket-ul clientilor nu poate fi mai mare de 10)
	int clienti[10];
	// initial nu avem niciun client
	for(int i = 0; i < 9; i++)
		clienti[i] = 0;
	
	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
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

					// trimitem lista cu clientii deja conectati catre clientul nou conectat
					memset(buffer, 0, BUFLEN);
					int buffer_pos = 0;
					for(int j = 0; j < 9; j++){
						if(clienti[j] == 1){
							buffer[buffer_pos] = (char)(j + 48);
							buffer_pos++;
							buffer[buffer_pos] = ';';
							buffer_pos++;
						}
					}
					buffer[buffer_pos] = '\n';
					buffer_pos++;
					buffer[buffer_pos] = '\0';
					buffer_pos++;
					n = send(newsockfd, buffer, strlen(buffer), 0);
					DIE(n < 0, "send");
					
					// ii informam pe ceilalti clienti de introducerea noului client
					for(int j = 0; j < 9; j++){
						memset(buffer, 0, BUFLEN);
						buffer[0] = (char) newsockfd + 48;
						memcpy(buffer + 1, " s-a conectat.\n", 16);
						if(clienti[j] == 1){
							n = send(j, buffer, strlen(buffer), 0);
							DIE(n < 0, "send");
						}
					}
					// adaugam noul client in lista
					clienti[newsockfd] = 1;

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
						clienti[i] = 0;
						memset(buffer, 0, BUFLEN);
						buffer[0] = (char) i + 48;
						memcpy(buffer + 1, " a iesit acum.\n", 16);
						// ii informam pe ceilalti clienti de scoaterea clientului
						for(int j = 0; j < 9; j++){
							if(clienti[j] == 1){
								n = send(j, buffer, strlen(buffer), 0);
								DIE(n < 0, "send");
							}
						}

					} else {
						printf ("S-a primit de la clientul de pe socketul %d mesajul: %s\n", i, buffer);
					}
				}
			}
		}
	}

	close(sockfd);

	return 0;
}
