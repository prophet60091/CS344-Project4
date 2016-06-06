//
// Created by Robert on 6/3/2016.
//

#ifndef PROJECT4_OTP_ENC_D_H
#define PROJECT4_OTP_ENC_D_H


typedef struct {
    char * msg;
    char * key;
} crypt;
int receiver(int sockfd, char  *msg, size_t msgBytes);
void error(char *msg, int severity);
int start_server(int port, int cc);
char * _decrypt(char * msg, char * key);
int sender(int socket, char *msg);
int _read_message(FILE *fpFILE, FILE *fpKEY, crypt *msg);
int process_message(char * fileName, char *keyName, char ** result);


#endif //PROJECT4_OTP_ENC_D_H
