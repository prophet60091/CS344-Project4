//
// Created by Robert on 6/3/2016.
//

#ifndef PROJECT4_OTP_ENC_H
#define PROJECT4_OTP_ENC_H

int make_connection(char* port);
int receiver(int sockfd, char  **msg, size_t msgBytes);
int authorize(int socket);
#endif //PROJECT4_OTP_ENC_H
