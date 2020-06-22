#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);

    // Ex 1: GET dummy from main server
    
    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/dummy", NULL, NULL, 0);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    
    // Ex 2: POST dummy and print response from main server
    
    char **body_data = NULL;
    body_data = calloc(3, 4);
    body_data[0] = calloc(6, 1);
    strcpy(body_data[0],"ab=cd");
    body_data[1] = calloc(6, 1);
    strcpy(body_data[1],"ef=gh");
    body_data[2] = calloc(6, 1);
    strcpy(body_data[2],"ij=kl");
    message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/dummy", "application/x-www-form-urlencoded", body_data, 3, NULL, 0);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    
    // Ex 3: Login into main server
    int login = 0;
    char **login_body_data = NULL;
    login_body_data = calloc(2, 4);
    login_body_data[0] = calloc(17, 1);
    strcpy(login_body_data[0],"username=student");
    login_body_data[1] = calloc(17, 1);
    strcpy(login_body_data[1],"password=student");
    message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/auth/login", "application/x-www-form-urlencoded", login_body_data, 2, NULL, 0);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    if(strstr(response, "HTTP/1.1 200 OK")){
        login = 1;
    }
    // Ex 4: GET weather key from main server
    char** cookies = calloc(1, 4);
    char* start = strstr(response, "Cookie: ");
    char* end = strstr(response, "Date");
    cookies[0] = ( char * )calloc( end - 2 - start - 8 + 1, 1);
    memcpy(cookies[0], start + 8, end - 2 - (start + 8));
    //puts(cookies[0]);
    //printf("------------\n%s", cookies[0]);
    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/weather/key", NULL, cookies, 1);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    
    // BONUS 1: make the main server return "Already logged in!"
    message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/auth/login", "application/x-www-form-urlencoded", login_body_data, 2, NULL, 0);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    if(strstr(response, "HTTP/1.1 200 OK") && login == 1){
        printf("Already logged in!\n");
    }
    
    // Ex 5: Logout from main server
    
    message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", "/api/v1/auth/logout", NULL, NULL, 0);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    
    // free the allocated data at the end!
    
    free(body_data[0]);
    free(body_data[1]);
    free(body_data[2]);
    free(body_data);
    
    free(login_body_data[0]);
    free(login_body_data[1]);
    free(login_body_data);
    
    free(cookies[0]);
    free(cookies);
    
    return 0;
}
