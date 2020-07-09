#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
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
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 3) {
		usage(argv[0]);
	}

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds
	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_aton(argv[1], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	// se adauga fd pentru citire in multimea read_fds
	// se adauga noul file descriptor (socketul pe care se primesc mesaje) in multimea read_fds
	FD_SET(0, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	// get = 1, put = 2, delete = 3
	int last_msg_value = 0;
	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(0, &tmp_fds)) {		
			// se citeste de la tastatura
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);
			if (strncmp(buffer, "quit", 4) == 0) {
				break;
			}
			char response[BUFLEN];
			memset(response, 0 ,BUFLEN);
			int no_par;
			char par[10][100];
			extract_parameters(buffer, par, & no_par);
			if(strcmp(par[0], "get") == 0) {
				strcat(response, "GET ");
				strcat(response, par[1]);
				strcat(response, " HTTP/1.1\r\n\r\n");
			}
			if(strcmp(par[0], "put") == 0) {
				strcat(response, "PUT ");
				strcat(response, par[1]);
				strcat(response, " HTTP/1.1\r\n\r\n");
				strcat(response, par[2]);
				last_msg_value = 2;
			}
			if(strcmp(par[0], "delete") == 0) {
				strcat(response, "DELETE ");
				strcat(response, par[1]);
				strcat(response, " HTTP/1.1\r\n\r\n");
				last_msg_value = 3;
			}
			// se trimite mesaj la server
			n = send(sockfd, response, strlen(response), 0);
			DIE(n < 0, "send");
		}
		if (FD_ISSET(sockfd, &tmp_fds)) {
			// receptionez si afisez
			memset(buffer, 0, BUFLEN);
			int r = recv(sockfd, buffer, BUFLEN, 0);
			if(r < 0){
				perror("recv error in client");
			}
			if(r == 0)
				break;
			printf("CONTINUT MESAJ PRIMIT:\n");
			printf("%s", buffer);
			if(last_msg_value == 2 && strncmp(buffer, "HTTP/1.1 404 Not Found", 23) == 0) {
				last_msg_value = 0;
				printf("Pagina cautata nu exista\n");
			}
			if(last_msg_value == 3 && strncmp(buffer, "HTTP/1.1 404 Not Found", 23) == 0) {
				last_msg_value = 0;
				printf("Pagina dorita nu exista\n");
			}
			printf("\n");
			// pentru put
			if(last_msg_value == 2 && strncmp(buffer, "HTTP/1.1 200 OK", 15) == 0) {
				last_msg_value = 0;
				printf("Pagina a fost salvata\n");
			}
			// pentru delete
			if(last_msg_value == 3 && strncmp(buffer, "HTTP/1.1 200 OK", 15) == 0) {
				last_msg_value = 0;
				printf("Pagina a fost stearsa\n");
			}
			fflush(stdout);
			
		}
	}

	close(sockfd);

	return 0;
}
