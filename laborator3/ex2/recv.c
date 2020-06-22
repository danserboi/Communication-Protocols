#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

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
int main(void)
{
	msg r;
	int i, res;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	
	int ok = 0;

	for (i = 0; i < COUNT; i++) {
		/* wait for message */
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
		
		int my_parity = parity(r);
		int s_parity = r.payload[0];
		if(my_parity == s_parity){
			ok++;
		}

		/* send dummy ACK */
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
	}

	printf("[RECEIVER] Finished receiving..\n");
	printf("%d", ok);
	return 0;
}
