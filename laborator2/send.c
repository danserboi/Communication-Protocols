#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000
// inline function to swap two numbers
void swap(char *x, char *y) {
  char t = *x; *x = *y; *y = t;
}

// function to reverse buffer[i..j]
char* reverse(char *buffer, int i, int j)
{
  while (i < j)
    swap(&buffer[i++], &buffer[j--]);

  return buffer;
}

// Iterative function to implement itoa() function in C
char* itoa(int value, char* buffer, int base)
{
  // invalid input
  if (base < 2 || base > 32)
    return buffer;

  // consider absolute value of number
  int n = abs(value);

  int i = 0;
  while (n)
  {
    int r = n % base;

    if (r >= 10) 
      buffer[i++] = 65 + (r - 10);
    else
      buffer[i++] = 48 + r;

    n = n / base;
  }

  // if number is 0
  if (i == 0)
    buffer[i++] = '0';

  // If base is 10 and value is negative, the resulting string 
  // is preceded with a minus sign (-)
  // With any other base, value is always considered unsigned
  if (value < 0 && base == 10)
    buffer[i++] = '-';

  buffer[i] = '\0'; // null terminate string

  // reverse the string and return it
  return reverse(buffer, 0, i - 1);
}
int main(int argc,char** argv){
  init(HOST,PORT);
  msg t;

  //SEND FILENAME
  char filename[] = "input_file";
  //Send dummy message:
  /*
  printf("[send] Sending dummy...\n");
  sprintf(t.payload,"%s", "This is a dummy.");
  t.len = strlen(t.payload)+1;
  send_message(&t);
  */
  printf("[send] Sending filename...\n");
  t.len = strlen(filename);
  strcpy(t.payload, filename);
  send_message(&t);
  // Check response:
  if (recv_message(&t)<0){
    perror("Receive error ...");
    return -1;
  }
  else {
    printf("[send] Got reply with payload: %s\n", t.payload);
  }


  //SEND LENGTH
  msg t_length;
  printf("[send] Sending length...\n");
  int fd = open(filename, O_RDONLY);
  int length = lseek(fd, 0, SEEK_END);
  //printf("%d", length);
  char str [MAX_LEN];
  itoa(length, str, 10);
  strcpy(t_length.payload, str);
  //printf("%s", str);
  t_length.len = strlen(t_length.payload);
  send_message(&t_length);
    // Check response:
  if (recv_message(&t_length)<0){
    perror("Receive error ...");
    return -1;
  }
  else {
    printf("[send] Got reply with payload: %s\n", t.payload);
  }
  //SEND MESSAGE
  lseek(fd, 0, SEEK_SET);
  msg packet;
  while(length > 0){
    packet.len = read(fd, packet.payload, MAX_LEN);
    length -= packet.len;
    send_message(&packet);
    if (recv_message(&t_length)<0){
    perror("Receive error ...");
    return -1;
    }
  }

  return 0;
}
