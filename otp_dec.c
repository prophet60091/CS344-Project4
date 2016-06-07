//
// Created by Robert on 5/30/2016.
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <strings.h>
#include "otp_dec.h"
char * pgrmIDENT = "dec";

void error(char *msg)
{
    perror(msg);
    exit(2);

}
//Function to make conection to a port
//@params port number as a string
int make_connection(char* port){

    int sockfd;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // use ipv4, or 6 and type of socket, TCP

    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");

    if (server == NULL) {
        error("ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); // clear the garbage

    serv_addr.sin_family = AF_INET; //TCP

    //assign the struct the proper values
    bcopy(server->h_addr_list[0],
          (char *)&serv_addr.sin_addr.s_addr,
          (size_t)server->h_length);
    serv_addr.sin_port = htons((uint16_t)atoi(port));

    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;

    //adapted a bit from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
    // adapted from http://www.linuxhowtos.org/data/6/client.c
    // OSU Lecture

};

//Receiving Function
//@params the socket int, pointer to the message, sizeof the message
// returns the result of the read
int receiver(int sockfd, char  **msg, size_t msgBytes){

    int m =0;
    int fullSize = (int)msgBytes;
    int chunk = BUFSIZ; // usually 1024, depending on system

    *msg = realloc(*msg, (sizeof(char) *msgBytes));

    // no chunking required!
    if (fullSize < chunk){
        chunk = fullSize;
    }

    //read everything
    do{
        m+= read(sockfd, *msg+m, (size_t)chunk);

        //our last piece is smaller than BUFSIZ. so dont read BUFSIZ
        if (fullSize - m < chunk){
            chunk = fullSize-m;
        }

    }while(m < msgBytes);

    return m;
}

int main(int argc, char *argv[]) {
    int x, n;
    char * msgBuffer = calloc(BUFSIZ, sizeof(char));
    char * msgSize = calloc(8, sizeof(char));
    char * newPort = calloc(8, sizeof(char));

    //check that we have enough args
    if (argc < 3) {
        fprintf(stderr, "usage <%s> [filename] [key filename] [port number]\n", argv[0]);
        exit(0);
    }

    //establish connection
    x = make_connection(argv[3]);

    if (x < 0)
        error("Connection failed on port");

    //announce who you are, program
    n = write(x, pgrmIDENT, 3);  //send pgrm IDENT
    if (n < 0){
        error("Sending file name failed:");
    }

    //get  new port assignment
    n =0;
    n= receiver(x, &newPort, 8);

    // error if we didnt receive the total.
    if (n < 8){
        fprintf(stdout, "Failed getting a new port: %i\n", n);
    }


    //hang up dial new connection
    close(x);
    x = make_connection(newPort);

    //Send the name of the file to be encrypted
    n = write(x, argv[1], 100);  //send file name
    if (n < 0){
        error("Sending file name failed:");
    }

    //send the name of the key file to be encrypted.
    n= write(x, argv[2], 100); // send key file name
    if (n < 0){
       error("Sending key name failed:");
    }

    // Receive the size of the incoming encrypted file.
    n= receiver(x, &msgSize, 8);
    if (n < 8){
        error("Didn't receive all the bytes for msgsize");
    }

    if(atoi(msgSize) > 0) { // message has data
        // receive the message based on the previous
        n = receiver(x, &msgBuffer, (size_t) atoi(msgSize));

        // error if we didnt receive the total.
        if (n < (size_t) atoi(msgSize)) {
            fprintf(stdout, "Didn't receive all the bytes in the message: %i\n", n);
            error("Didn't receive all the bytes");
        }

        //out put the info to stdout
        fprintf(stdout, "%s", msgBuffer);
    }

    //FREEDOOOOMM!!
    free(msgBuffer);
    free(msgSize);
    close(x);

}
