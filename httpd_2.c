/*
AUTHOR: Abhijeet Rastogi (http://www.google.com/profiles/abhijeet.1989)

This is a very simple HTTP server. Default port is 10000 and ROOT for the server is your current working directory..

You can provide command line arguments like:- $./a.aout -p [port] -r [path]

for ex. 
$./a.out -p 50000 -r /home/
to start a server at port 50000 with root directory as "/home"

$./a.out -r /home/shadyabhi
starts the server at port 10000 with ROOT as /home/shadyabhi

*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include <string.h>

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
void respond(int);

void usage() {
    printf("\n");
    printf("Usage: httpd_2 -r <path> -p <port no> -?\n\n");
    printf("\t-r <path>\tPath to files.\n");
    printf("\t-p <port no>\tPort number.\n");
    printf("\t-?|h\t\tHelp.\n");
    printf("\n");

}

void sendTo(int fd, char *data) {
    int len;

    len=strlen(data)+1;

    send(fd, data, len, 0);
}

void header(int fd,int contentLength) {
    char buffer[255];  // completely arbritray, should be enough.

    sendTo(fd, "Server: Test/Testing\n");
    sendTo(fd, "Content-Type: text/plain\n");
    if(contentLength > 0) {
        sprintf(buffer,"Content-Length: %d\n", contentLength);
        sendTo(fd, buffer);
    }
    sendTo(fd,"\n\n");
}

int query(char *q) {
    char *tmp;
    char *tmp1;

    char *room;
    char *device;
    char *param;

    int rc;   // http status code

    printf("Query: %s\n",q);

    room = strtok(q,"/");

    printf("room:%s\n", room);

    device = strtok(NULL,"/?");

    printf("device:%s\n", device);

    param = strtok(NULL,"/");
    if( param != (char *)NULL) {
        printf("param:%s\n", param);

        tmp = strchr( param, '=');
        if(tmp == (char *)NULL) {
            printf("No =\n");
        } else {
            tmp1=strrchr( param, '\0' );
            if( tmp+1 == tmp1 ) {
                char *p;
                printf("No value\n");

                p=strtok(param,"=");
                printf("Query, sortof\n");
                printf("select %s from %s\n",p,room);
            } else {
                char *p;
                char *v;

                printf("Value\n");

                p=strtok(param,"=");
                v=strtok(NULL,"=");
                printf("select %s from %s where %s='%s'\n",p,room,p,v);
            }
        }
    }

    rc = 200;

    return(rc);
}

int update(char *u) {
    char *room;
    char *device;
    char *param;
    int rc=200;

    printf("%s\n",u);

    room = strtok(u,"/");

    device = strtok(NULL,"/?");

    param = strtok(NULL,"/");

    printf("Room  : %s\n",room);
    printf("Device: %s\n",device);
    printf("Param : %s\n",param);

    return(rc);

}

int main(int argc, char* argv[]) {
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;    
    
    //Default Values PATH = ~/ and PORT=10000
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT,"10000");

    int slot=0;

    //Parsing the command line arguments
    while ((c = getopt (argc, argv, "hp:r:")) != -1) {
        switch (c) {
            case 'r':
                ROOT = malloc(strlen(optarg));
                strcpy(ROOT,optarg);
                break;
            case 'p':
                strcpy(PORT,optarg);
                break;
            case 'h':
            case '?':
                usage();
                exit(1);
            default:
                exit(1);
        }
    }
    
    printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i=0; i<CONNMAX; i++) {
        clients[i]=-1;
    }
    startServer(PORT);

    // ACCEPT connections
    while (1) {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0) {
            error ("accept() error");
        } else {
            if ( fork()==0 ) {
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }

    return 0;
}

//start server
void startServer(char *port) {
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo( NULL, port, &hints, &res) != 0) {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next) {
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL) {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 ) {
        perror("listen() error");
        exit(1);
    }
}

//client connection
void respond(int n) {
    char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;

    memset( (void*)mesg, (int)'\0', 99999 );

    rcvd=recv(clients[n], mesg, 99999, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
        // receive socket closed 
    else if (rcvd==0) {
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    } else {    // message received
        printf("==========================\n");
        printf("%s", mesg);
        printf("\n==========================\n");

        reqline[0] = strtok (mesg, " \t\n");
        reqline[1] = strtok (NULL, " \t");
        
        if ( strncmp(reqline[0], "GET\0", 4)==0 ) {
            reqline[2] = strtok (NULL, " \t\n");
            
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 ) {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            } else {
                if ( strncmp(reqline[1], "/\0", 2)==0 )
                    reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);

                switch(query( reqline[1] )) {
                    case 200:
                        sendTo(clients[n], "HTTP/1.0 200 OK\n");
                        header(clients[n],4);  // Length of string below
                        sendTo(clients[n], "22\n\n");
                        break;
                    case 404:
                        sendTo(clients[n], "HTTP/1.0 404 Not Found\n");
                        header(clients[n], 0);
                        break;
                }

                printf("file: %s\n", path);

                /*
                //FILE FOUND
                if ( (fd=open(path, O_RDONLY))!=-1 )    {
                    send(clients[n], "HTTP/1.0 200 OK\n", 17, 0);
                    header(clients[n]);

                    while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
                        write (clients[n], data_to_send, bytes_read);
                } else {
                    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                }
                */
            }

        } else if ( strncmp(reqline[0], "HEAD\0", 5)==0 ) {
            printf("HEAD\n");
            sendTo(clients[n], "HTTP/1.0 200 OK\n");
            header(clients[n],0);
        } else if ( strncmp(reqline[0], "POST\0", 5)==0 ) {
            printf("POST\n");
            switch( update(reqline[1])) {
                case 200:
                    sendTo(clients[n], "HTTP/1.0 200 OK\n");
                    header(clients[n],0);
                    break;
                case 404:
//                    write(clients[n], "HTTP/1.0 404 Not Found\n", 23);
                    sendTo(clients[n], "HTTP/1.0 404 Not Found\n");
                    header(clients[n],0);
                    break;
            }
        }
    }

    //Closing SOCKET
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
}
