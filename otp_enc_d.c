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
#include <string.h>
#include <sys/stat.h>


typedef struct {
    char * msg;
    char * key;
} crypt;


void error(char *msg)
{
    perror(msg);

}

int start_server(char * port){

    int sockfd, optval, ears;
    struct sockaddr_in serv_addr;

    //establish socket type
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

//    // set SO_REUSEADDR on a socket to true (1):
//    optval = 1;
//    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // clear out the bytes
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //set serv_addr
    serv_addr.sin_family = AF_INET; //STREAM
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t)atoi(port)); // wat port

    //check for binding
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("SERVER ERROR on binding");


    ears = listen(sockfd, 5);
    if(ears  < 0 ){
        error("I can't hear you! Lalalalalala");
    }else{
        fprintf(stdout, "Listening on %s\n", port);
    }

    return sockfd;

//    while(1) {
//
//        listen(sockfd, 5);
//
//        clilen = sizeof(cli_addr);
//
//        nsocket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
//
//        if (nsocket  < 0)
//            error("SERVER ERROR on Accept");
//
//        message = (char *) malloc(msgBytes);
//        bzero(message, msgBytes);
//
//        while (1) {
//
//            m = receiver(nsocket, message, msgBytes);
//            bzero(message, msgBytes);
//
//            // if quit is sent
//            if (m <= 0) {
//                close(nsocket);
//                free(message);
//                break;
//            }
//
//        }
//        close(nsocket);
//        free(message);
//    }

    //return nsocket;
    //adapted a bit from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
    // adapted from http://www.linuxhowtos.org/data/6/client.c
}

int * gen_key(size_t size){
    int i;
    FILE * fp;
    int * key = malloc(sizeof(int) * size+1);
    int keySize =0;
    int seed =0;

    // read from dev urandom because better random seed!
    if((int)(fp = fopen("/dev/urandom" , "r" )) < 0) { // opening fails

        error("Couldn't open random " );
        return NULL;
    }

    fread(&seed, sizeof(int), 1, fp);
    fclose(fp);

    srand((unsigned)seed);

    //get the actual size of that key generated in case it was too short
    for(i = 0; i < size; i++){
            key[i] = rand();
    }

    return key;

}


char * encrypt(char * msg, char * key){
    int i;
    int msgLength = strlen(msg);
    int res;
    char * encMsg = malloc(sizeof(char *)* msgLength+1);

    for(i =0; i < msgLength; i++){


        res = key[i] + msg[i]; // key + our ascii value

        if( res > 126){     //take care of all ascii vals
            res = res - 126 + 32;
        }

        encMsg[i] = (char)res;
    }
    encMsg[i]= '\n';

    return encMsg;
}


char * decrypt(char * msg, char * key){
    int i;
    int msgLength = strlen(msg);
    int res;
    char * encMsg = malloc(sizeof(char *)* msgLength+1);

    for(i =0; i < msgLength; i++){

        res = msg[i] - key[i];


        if(res < 0){
            res = res + 126 - 32;
        }

        encMsg[i] = (char)res;
    }
    encMsg[i] = '\0';
    return encMsg;
}

//Receiving Function
//@params the socket int, pointer to the message, sizeof the message

// returns the result of the read
int receiver(int sockfd, char  *msg, size_t msgBytes){
    int m =0;
    while(m < msgBytes){
        m = read(sockfd, msg, msgBytes);

        if (m < 0){
            printf("SERVER ERROR receiving from socket");
            return m;
        }
    }
    return m;

}

int read_message(FILE * fpFILE, FILE * fpKEY, crypt * msg ){

    size_t s= 0;
    size_t cur_fileLength = 1;
    size_t cur_keyLength = 1;
    size_t end = 0;
    int result =0;

    msg->msg= malloc(BUFSIZ);
    msg->key= malloc(BUFSIZ);

    result = getline(&msg->msg, &s, fpFILE );
     if (result < 0){
         error("failed reading from file");
         return result;
     }

    result = getline(&msg->key, &s, fpKEY );
    if (result < 0){
        error("failed reading from file");
        return result;
    }

    //pop off the newline character
    msg->msg[strlen(msg->msg) -1] = NULL;
    msg->key[strlen(msg->key) -1] = NULL;

    return result;
}


int process_message(char * fileName, char *keyName, char ** result){
    FILE * fpFile;
    FILE * fpKey;
    int gets;
    crypt * msg;

    fpFile = fopen(fileName, "r");
    fpKey = fopen(keyName, "r");

    if(fpFile  < 0 || fpKey < 0){
        error("file not found");
        return -1;
    }

    msg = malloc(sizeof(crypt));

    gets = read_message(fpFile, fpKey, msg);
    if(gets < 0){
        error("Couldnt read message");
        return -1;
    }

    *result = malloc(sizeof(msg->msg));
    *result  =  encrypt(msg->msg, msg->key);


    free(msg);

    fclose(fpFile);
    fclose(fpKey);

    return 0;
}


int main(int argc, char *argv[])
{
    int socket, accept_socket, result, ears;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr, client_fd;

    //int * key = gen_key(256);

    int fileNameSize;
    char keyNameSize[1];
    char portSize[1];

    char fileName[1024];
    char keyName[1024];
    char * encrypted;
    char port[10];

    if (argc < 1) {
        fprintf(stderr,"usage is <%s>  [port number]\n", argv[0]);
        exit(0);
    }


   socket = start_server(argv[1]);

    if(socket < 0)
        error("no socket");

    //loop to accept incoming connections;
        clilen = sizeof(cli_addr);

    while ((accept_socket = accept(socket, (struct sockaddr *) &cli_addr, &clilen)) >=0){
        if (accept_socket < 0) {
            error("SERVER ERROR on Accept");
        }else{

            fprintf(stdout, "client connected...\n");
        }

        receiver(accept_socket, fileName, 100);
        receiver(accept_socket, keyName, 100);

        if(( process_message(fileName, keyName, &encrypted )) < 0)
           error("couldn't process message");

        //fprintf(stdout, "Received: %s - %s\n", fileName, keyName);
        write(accept_socket, (char *)strlen(encrypted), 4);
        write(accept_socket, encrypted, strlen(encrypted));

    }


    close(accept_socket);


//     if(( process_message("plaintext1", "testKey", &encrypted )) < 0)
//            error("couldn't process message");

    fprintf(stdout, "%s\n", encrypted);

    free (encrypted);


    close(socket);
}
