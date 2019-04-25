//
//  requests.h
//  requests
//
//  Created by hao on 2019/4/25.
//  Copyright © 2019 hao. All rights reserved.
//

/**
 * Protocol: HTTP1.1, TCP, IPv4
 */


#ifndef _requests_h
#define _requests_h

#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>

#define FAMILY AF_INET
#define PORT 80
#define TYPE SOCK_STREAM
#define HTTP_VERSION "HTTP/1.1"

#define MAXLINE 1024

struct request {
    char *method;
    char **headers;
    char **data;
    char **params;
};

struct url {
    char *host;
    char *path;
};

struct response {
    int status_code;
    char *reason;
    char **headers;
    char *content;
};

#endif /* requests_h */
