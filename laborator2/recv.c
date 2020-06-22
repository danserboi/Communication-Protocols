#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001


int main(int argc,char** argv){
  msg r;
  init(HOST,PORT);

  //RECEIVE FILENAME
  if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
  }
  printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);

  // Send ACK:
  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);
  printf("[recv] ACK sent\n");


  //RECEIVE LENGTH
  msg r2;
  if (recv_message(&r2)<0){
    perror("Receive message");
    return -1;
  }
  char str_length[MAX_LEN];
  strcpy(str_length, r2.payload);
  int length = atoi(str_length);
  printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r2.payload);

  // Send ACK:
  sprintf(r2.payload,"%s", "ACK");
  r2.len = strlen(r2.payload) + 1;
  send_message(&r2);
  printf("[recv] ACK sent\n");

  //RECEIVE MESAGE
  
  int fd = open("output_file", O_CREAT | O_WRONLY, 0777);
  while(length > 0){
  	  //RECEIVE PACKET
	  if (recv_message(&r)<0){
	    perror("Receive message");
	    return -1;
	  }
	  //printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);
	  write(fd, r.payload, r.len);
	  length -= r.len;
	  
	  
	  // Send ACK:
	  sprintf(r.payload,"%s", "ACK");
	  r.len = strlen(r.payload) + 1;
	  send_message(&r);
	  //printf("[recv] ACK sent\n");
	  
  }
  
  return 0;
}
