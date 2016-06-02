//
// Created by Robert on 5/31/2016.
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
#include <string.h>


char action[6];
char *name;
char *message;
size_t nameBytes = 11;
size_t msgBytes = 512;


void error(char *msg)
{
    perror(msg);

}

char * gen_key(size_t size){
    int i;
    FILE * fp;
    char * key = malloc(sizeof(char) * size+1);
    int keySize =0;
    int seed =0;
    char ascii = NULL;

    char * randomchar = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ "};

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

        key[i] = randomchar[random()%27];
    }

    return key;

}

int main(int argc, char *argv[])
{
    int i =0;
    if (argc <= 1) {
        fprintf(stderr,"usage keygen <key length>.\n");
        exit(0);
    }

    char * key = gen_key((size_t)atoi(argv[1]));


    fprintf(stdout, "%s", key);


    fprintf(stdout, "\n");
}
