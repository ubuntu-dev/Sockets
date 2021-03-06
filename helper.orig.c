/*
   HELPER.C
   ========
   (c) Paul Griffiths, 1999
Email: mail@paulgriffiths.net

Implementation of sockets helper functions.

Many of these functions are adapted from, inspired by, or
otherwise shamelessly plagiarised from "Unix Network
Programming", W Richard Stevens (Prentice Hall).

*/

#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t         n, rc;
    char            c, *buffer;
    int count = 0;

    buffer = vptr;

//    printf("Readline.\n");
    for (n = 1; n < maxlen; n++) {

        if((rc=recv(sockd, &c, 1, 0)) == 1)  {
            *buffer++ = c;

<<<<<<< Updated upstream
=======
#ifdef DEBUG
            if( c >= 0x20 && c <=0x7f) {
                printf(":%c: <%02x>",c, c);
            } else {
                printf(":.: <%02x>", c);
            }
>>>>>>> Stashed changes
            count++;
            if (c == '\n') {
                printf("\n");
                break;
            }
#endif
        } else if (rc == 0) {
            printf("B\n");
            if (n == 1)
                return 0;
            else
                break;
        } else {
            // printf("C\n");
            if (errno == EINTR)
                continue;
            return -1;
        }
    }

    *buffer = 0;
    return n;
}


/* Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t          nleft;
    ssize_t         nwritten;
    const char     *buffer;

    buffer = vptr;
    nleft = n;

//    printf("Writeline\n");
    while (nleft > 0) {
        if ((nwritten = write(sockd, buffer, nleft)) <= 0) {
            if (errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }
        nleft -= nwritten;
        buffer += nwritten;
    }

    return n;
}
