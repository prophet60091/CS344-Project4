//
// Created by Robert on 6/3/2016.
//

#ifndef PROJECT4_OTP_ENC_D_H
#define PROJECT4_OTP_ENC_D_H
typedef struct {
    char * msg;
    char * key;
} crypt;

void error(char *msg);
int start_server(char * port);
char * encrypt(char * msg, char * key);
char * decrypt(char * msg, char * key);
int receiver(int sockfd, char  *msg, size_t msgBytes);
int read_message(FILE * fpFILE, FILE * fpKEY, crypt * msg );
int process_message(char * fileName, char *keyName, char ** result);


#endif //PROJECT4_OTP_ENC_D_H
