//
// Created by Robert on 5/30/2016.
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/errno.h>
#include <strings.h>
#include "otp_enc.h"


//Sending Files
//@params the socket int, string file contents, sizeof the message
// send the file
//void sendfile( int sockfd, string file, unsigned long size){
//    int m, s;
//    unsigned long offset = 0;
//
//    std::cout << "TOTAL sending " << file.size() << std::endl;
//
//    //SEND THE WHOLE FILE
//    while (offset < file.size()) {
//
//        m = send(sockfd, file.c_str()+offset, file.size()-offset,0);
//        std::cout << "sending " << m << std::endl;
//        if (m <= 0) break;
//        offset += m;
//    }// ADAPTED FROM http://stackoverflow.com/questions/15176213/read-the-whole-file-and-send-it-via-sockets
//
//}

void error(char *msg)
{
    perror(msg);
    exit(2);

}


int make_connection(char* port){

    int sockfd, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    int is = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");

    if (server == NULL) {
        error("ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    bcopy(server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          (size_t)server->h_length);
    serv_addr.sin_port = htons((uint16_t)atoi(port));

    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

//    while(is == 0){
//
//        is = get_user();
//    }

    return sockfd;

};

//Receiving Function
//@params the socket int, pointer to the message, sizeof the message

// returns the result of the read
int receiver(int sockfd, char  *msg, size_t msgBytes){
    int m =0;
    msg = malloc(sizeof(char) *msgBytes);
    while(m < msgBytes){
        m = read(sockfd, msg, msgBytes);

        if (m < 0){
            printf("SERVER ERROR receiving from socket");
            return m;
        }
    }
    return m;

}

int main(int argc, char *argv[]) {
    int x, sent, rcvd;
    char * msgBuffer;
    char * msgSize;

    if (argc < 3) {
        fprintf(stderr, "usage <%s> [filename] [key filename] [port number]\n", argv[0]);
        exit(0);
    }


    x = make_connection(argv[3]);

    if (x < 0)
        error("Connection failed on port");

    send(x, argv[1], 100, MSG_DONTWAIT);  //send file name
    send(x, argv[2], 100, MSG_DONTWAIT); // send key file name
    receiver(x, msgSize, 8);
    receiver(x, msgBuffer, (size_t)msgSize);
    fprintf(stdout, "%s", msgBuffer);

    close(x);
//    rcvd = receiver(x, buffer, 1000);
//    if (rcvd < 0)
//        error("ERROR writing to socket");
}
