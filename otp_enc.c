//
// Created by Robert on 5/30/2016.
//

int make_connection(char* port){

    int sockfd, n, portno;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(port); // may not be needed, leftover from other program
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname("localhost");

    if (server == NULL) {
        fprintf(stderr,"ERROR, host unavailable\n");
        exit(0);

    }else{

        bzero((char *) &serv_addr, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;

        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(portno);

        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
            error("ERROR connecting");
        }else{

            printf("Listening on port %i", portno);
        }
    }

    return 0;

};

