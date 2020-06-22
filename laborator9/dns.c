// Protocoale de comunicatii
// Laborator 9 - DNS
// dns.c

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int usage(char *name)
{
	printf("Usage:\n\t%s -n <NAME>\n\t%s -a <IP>\n", name, name);
	return 1;
}

// Receives a name and prints IP addresses
void get_ip(char *name)
{
	struct addrinfo hints, *result = NULL, *p = NULL;

	// TODO: set hints
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	// TODO: get addresses
	int r = getaddrinfo(name, "http", &hints, &result);
	if (r < 0)
	{
		printf("%s", gai_strerror(r));
	}
	// TODO: iterate through addresses and print them
	char buf[100];
	p = result;
	int i = 1;
	while (p)
	{
		if (p->ai_family == AF_INET)
		{
			struct sockaddr_in *in = (struct sockaddr_in *)p->ai_addr;
			inet_ntop(p->ai_family, &in->sin_addr, buf, 100);
			printf("Adresa IP este: %s\n", buf);
			int socktype = p->ai_socktype;
			printf("Socktype este: %u\n", socktype);
			int protocol = p->ai_protocol;
			printf("Protocol este: %u\n", protocol);
			char* canon_name = p->ai_canonname;
			if(canon_name){
				printf("Canon Name este: %s\n", canon_name);
			}
			short port = ntohs(in->sin_port);
			printf("Portul este: %hu\n", port);
			printf("Iteratie: %d\n", i);
		}
		else if (p->ai_family == AF_INET6)
		{
			struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)p->ai_addr;
			inet_ntop(p->ai_family, &in6->sin6_addr, buf, 100);
			printf("Adresa IP este: %s\n", buf);
			int socktype = p->ai_socktype;
			printf("Socktype este: %u\n", socktype);
			int protocol = p->ai_protocol;
			printf("Protocol este: %u\n", protocol);
			char* canon_name = p->ai_canonname;
			if(canon_name){
				printf("Canon Name este: %s\n", canon_name);
			}
			short port = ntohs(in6->sin6_port);
			printf("Portul este: %hu\n", port);
			printf("Iteratie: %d\n", i);
		}
		p = p->ai_next;
		i++;
	}
	// TODO: free allocated data
	freeaddrinfo(result);
}

// Receives an address and prints the associated name and service
void get_name(char *ip)
{
	int ret;
	struct sockaddr_in addr;
	char host[1024];
	char service[20];

	// TODO: fill in address data
	memset((char *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	inet_aton(ip, &addr.sin_addr);

	// TODO: get name and service
	ret = getnameinfo((struct sockaddr*) &addr, sizeof(addr), host, 1024, service, 20, 0);
	if(ret < 0){
		printf("%s", gai_strerror(ret));
	}
	// TODO: print name and service
	printf("HOST = %s, SERVICE = %s\n", host, service);
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		return usage(argv[0]);
	}

	if (strncmp(argv[1], "-n", 2) == 0)
	{
		get_ip(argv[2]);
	}
	else if (strncmp(argv[1], "-a", 2) == 0)
	{
		get_name(argv[2]);
	}
	else
	{
		return usage(argv[0]);
	}

	return 0;
}
