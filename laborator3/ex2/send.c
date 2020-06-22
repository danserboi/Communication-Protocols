#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

char bits(char character){
	char bits = 0;
	char mask = 1;
	for(char i = 0; i < 8; i++){
		if((mask&character) == 1){
			bits++;
		}
		character = character >> 1;
	}
	return bits;
}

int parity(msg p){
	int sum = 0;
	for(int i = 1; i <= p.len; i++){
		sum += bits(p.payload[i]);
	}
	return sum % 2;
}

int main(int argc, char *argv[])
{
	msg t;
	int i, res;

	memset(&t, 0, sizeof(msg));
	sprintf(t.payload + 1, "ana");
	t.payload[0] = (char)parity(t);
	t.len = strlen("ana") + 2;
	printf("%d", parity(t));


	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);

	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */
	
	for (i = 0; i < COUNT; i++) {
		/* cleanup msg */
		sprintf(t.payload + 1, "ana");
		t.payload[0] = (char)parity(t);
		t.len = strlen("ana") + 2;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}

	printf("[SENDER] Job done, all sent.\n");
		
	return 0;
}
