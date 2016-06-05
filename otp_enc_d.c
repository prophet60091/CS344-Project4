#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include "otp_enc_d.h"

void error(char *msg, int severity)
{
    perror(msg);

    if (severity > 1){
        exit(severity);
    }

}

//starts the server listening and binding to ports
// @ param the port number
int start_server(int port, int cc){

    int sockfd, optval, ears;
    struct sockaddr_in serv_addr;

    //establish socket type
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket", 2);

//    // set SO_REUSEADDR on a socket to true (1):
//    optval = 1;
//    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // clear out the bytes
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //set serv_addr
    serv_addr.sin_family = AF_INET; //STREAM
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t)port); // wat port

    //check for binding
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("SERVER ERROR on binding", 2);

    //strat listening
    ears = listen(sockfd, cc);
    if(ears  < 0 ){
        error("I can't hear you! Lalalalalala", 2);
    }else{
        fprintf(stdout, "Listening on %i\n", port);
    }

    return sockfd;

    //adapted a bit from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
    // adapted from http://www.linuxhowtos.org/data/6/client.c
    // OSU Lecture
}

// Encrypts the message based on the key provided
// @ param the message to be encrpted
// @ param the key (same format as above assumed to be at least length of msg)
char *_encrypt(char *msg, char *key){
    int i;
    int msgLength = strlen(msg);
    int res;
    char * encMsg = calloc((size_t)msgLength, sizeof(char));

    if(strlen(key) < msgLength)
        error("Invalid Key: too small", 1);

    for(i =0; i < msgLength; i++){

        res = key[i] + msg[i]; // key ascii val + our ascii value

        if( res > 126){     //take care of all printable ascii chars (wraps when outside of range)
            res = res - 126 + 32;
        }

        encMsg[i] = (char)res;
    }
    encMsg[i]= '\n';

    return encMsg;
}

// Decrypts the message based on the key provided
// @ param the message to be decrypted
// @ param the key (same format as above assumed to be at least length of msg)
char * decrypt(char * msg, char * key){
    int i;
    int msgLength = strlen(msg);
    int res;
    char * encMsg = malloc(sizeof(char)* msgLength+1);

    for(i =0; i < msgLength; i++){

        res = msg[i] - key[i];


        if(res < 0){
            res = res + 126 - 32;
        }

        encMsg[i] = (char)res;
    }
    encMsg[i] = '\n';
    return encMsg;
}

//Receiving Function
//@params the socket int, pointer to the message, sizeof the message
// returns the result of the read ie. the number of bytes read
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

//Reads the message into memory
//@params file pointer to the File to be encrypted
//@params file pointer to the Key file
//@params point to the strict where the msg and key pair will be stored
int _read_message(FILE *fpFILE, FILE *fpKEY, crypt *msg){

    size_t s= 0;
    int result= NULL;

    //clean bits for your enjoyment
    msg->msg= calloc(BUFSIZ, sizeof(char));
    msg->key= calloc(BUFSIZ, sizeof(char));

    //gather the goods
    result = getline(&msg->msg, &s, fpFILE );
     if (result < 0){
         error("failed reading from file", 227);
     }

    s= 0; //VIP!!! reset s to 0 for the reading the key

    //get the key
    result = getline(&msg->key, &s, fpKEY );
    if (result < 0){
        error("failed reading from file", 227);
        exit(227);
    }

    //pop off the newline character
    msg->msg[strlen(msg->msg) -1] = '\0'; // add a trailing 0 byte for file creation
    msg->key[strlen(msg->key) -1] = '\0';

    return result;
}

//Processes the messages, using read_message and encrypt
//@params the name of the file
//@params the name of the key
//@ where to store the result (will be dynamically sized)
int process_message(char * fileName, char *keyName, char **result){
    FILE * fpFile;
    FILE * fpKey;
    int gets;
    crypt * msg = malloc(sizeof(crypt));;

    fpFile = fopen(fileName, "r");
    fpKey = fopen(keyName, "r");

    if(fpFile  < 0 || fpKey < 0){
        error("file not found", 227);
        return -1;
    }

    // gather the message into memory
    gets = _read_message(fpFile, fpKey, msg);
    if(gets < 0){
        error("Couldn't read message", 666);
        return -1;
    }


    //get some clean bits and store the encrypted text
    *result = calloc(strlen(msg->msg), sizeof(char));
    *result  = _encrypt(msg->msg, msg->key);

    free(msg);

    fclose(fpFile);
    fclose(fpKey);

    return 0;
}

//sends the number of bytes until all sent
//@params the sockt on which to send
//@params pointer to the the message to be sent
int sender(int socket, char *msg){
    int n=0;
    int fullSize =(int)strlen(msg);
    int chunk = BUFSIZ;

    // no chunking required!
    if (fullSize < chunk){
        chunk = fullSize;
    }

    do{
        n+= write(socket, msg + n, (size_t)chunk);

        if (fullSize - n < chunk){
            chunk = fullSize-n;
        }
        fprintf(stdout, "sending...");
    }while(n < strlen(msg));
        fprintf(stdout, "sent");
    return n;

}


int main(int argc, char *argv[])
{
    int socket, newSocket, accept_socket, com_socket, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr, client_fd;
    char fileName[1024];
    char keyName[1024];
    char * encrypted;
    char eLength[8] ;
    char newPortString[8];
    int  newPort;
    pid_t pcessID= -5;
    pid_t wpid= -5;
    int status;
    clilen = sizeof(cli_addr);

    if (argc < 1) {
        fprintf(stderr,"usage is <%s>  [port number]\n", argv[0]);
        exit(0);
    }

    //establish the hookup channel
    socket = start_server(atoi(argv[1]), 5);

    if(socket < 0)
        error("no socket", 3);

    //loop to accept incoming connections;
    while ((accept_socket = accept(socket, (struct sockaddr *) &cli_addr, &clilen)) >=0){
        if (accept_socket < 0) {
            error("SERVER ERROR on Accept", 3);
        }else{

            fprintf(stdout, "client connected...\n");
        }

        //make sure all garbage is out
        memset(eLength, 0, sizeof(eLength));
        memset(fileName, 0, sizeof(fileName));
        memset(keyName, 0, sizeof(keyName));

        /// FIRST ESTABLISH A NEW COMMUNICATION PORT
        srand((unsigned)time(NULL)); // seed random
        newPort = atoi(argv[1]) + (rand() % 21 + 1); // newport starting point

        //Loop unitl we get a good port
        int i = 1;
        while ((newSocket = start_server(newPort, 1)) < 0){

            newPort = newPort +i; // base the new off of the last accepted FD (err socket descriptor)
            i++;
        };

        sprintf(newPortString, "%i", newPort); // gets the portnumber into a string
        //send that to the client
        if( (n=write(accept_socket, newPortString, 8)) < 8){
            fprintf(stdout, "only sent %i bytes", n);
            error("Sending Port: Didn't send enough bytes", 1);
        }

        close(accept_socket);

        pcessID = fork();
        printf("spawning processes..%i", pcessID);
        //partially adapted from lecture 9 cs344
        switch((int)pcessID){

            case -1:
                //it's in a bad state

                error("boom!", 5050505);
                exit(1);


            case 0:// WE'RE IN THE CHILD PROCESS

                com_socket = accept(newSocket, (struct sockaddr *) &cli_addr, &clilen);

                if (com_socket < 0) {
                    error("SERVER ERROR on Accept", 4);
                }else{

                    fprintf(stdout, "client connected...\n");
                }

                //NOW PROCESS MESSAGES AND THE LIKE

                if ((n = receiver(com_socket, fileName, 100)) < 0){
                    error("didnt receive file name", 2);
                }

                if ((n= receiver(com_socket, keyName, 100)) < 0){
                    error("didnt receive key file name", 2);
                }

                if(( n= process_message(fileName, keyName, &encrypted )) < 0)
                    error("couldn't process message", 2);


                sprintf(eLength, "%zu", strlen(encrypted)); // gets the length of the encrypted txt into a string
//
//                if (n != 0){
//                    error( "Something went wrong when getting the length\n", 1);
//                }

                if( (n=write(com_socket, eLength, 8)) < 8){
                    fprintf(stdout, "only sent %i bytes", n);
                    error("Writing Size: Didn't send enough bytes", 1);
                }

                if ((n = sender(com_socket, encrypted)) < 0){
                    error("Failed Sending", 1);
                }

                close(com_socket);
                free (encrypted);

                kill(pcessID, SIGKILL);

            default:
                // WERE IN THE PARENT
                //ADAPTED FROM **http://brennan.io/2015/01/16/write-a-shell-in-c/**//
                //Too elegant to pass up. it works really well
                //  were storing the result of waitpid using WUNTRACED, (reports its status whether stopped or not)
                // as long as the process didn't exit, or receive a signal, so it's waiting util that happens
                // when it does, we know that the child process is complete.
                    do {
                        wpid = waitpid(pcessID, &status, WUNTRACED);

                    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        }


    }

    close(socket);
}
