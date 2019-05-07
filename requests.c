//
//  requests.c
//  requests
//
//  Created by hao on 2019/4/25.
//  Copyright © 2019 hao. All rights reserved.
//

#include "requests.h"

void set_servaddr(const char *const hostname, struct sockaddr_in *servaddr_ptr) {
    bzero(servaddr_ptr, sizeof(struct sockaddr_in));
    servaddr_ptr->sin_family = FAMILY;
    servaddr_ptr->sin_port = htons(PORT);
    
    struct hostent *host_ptr;
    host_ptr = gethostbyname(hostname);
    if (host_ptr == NULL) {
        printf("DNS error\n");
        exit(-1);
    }
    void *src = *host_ptr->h_addr_list;
    void *dest = &servaddr_ptr->sin_addr;
    if (src == NULL) {
        exit(-1);
    }
    bcopy(src, dest, sizeof(struct in_addr));
    
    return;
}

void parse_url(const char *const url, struct url *url_ptr) {
    int count = 0;
    int i;
    int host_l = 0, host_r = 0; /* start of host, and start of path */
    for (i = 0; i < strlen(url); i++) {
        if (url[i] == '/' && count == 0) {
            count++;
            continue;
        }
        else if (url[i] == '/' && count == 1) {
            count++;
            host_l = i + 1;
            continue;
        }
        else if (url[i] == '/' && count == 2) {
            count++;
            host_r = i;
            break;
        }
    }
    /* copy the memory */
    void *dest, *src;
    if (count < 2) {
        exit(-1);
    } else if (count == 2) {
        src = url + sizeof(char) * host_l;
        host_r = strlen(url);
        url_ptr->host = malloc(host_r - host_l);
        dest = url_ptr->host;
        bcopy(src, dest, host_r - host_l);
        url_ptr->path = "/";
        return;
    }
    src = url + sizeof(char) * host_l;
    url_ptr->host = malloc(host_r - host_l);
    dest = url_ptr->host;
    bcopy(src, dest, host_r - host_l);
    
    src = url + sizeof(char) * host_r;
    url_ptr->path = malloc(strlen(url) - host_r);
    dest = url_ptr->path;
    bcopy(src, dest, strlen(url) - host_r);
    return;
}

void set_http_request(const char *const url, struct request *request_ptr, char *request_buff) {
    int count = 0; /* record the count of the request_buff */
    char *line_sep = "\r\n";
    
    struct url s_url;
    parse_url(url, &s_url);
    
    void *src, *dest;
    src = request_ptr->method;
    dest = request_buff + count;
    bcopy(src, dest, strlen(src));
    count += strlen(src);
    bcopy(" ", request_buff + count, 1);
    count++; /* to add a space between method and path */
    
    src = s_url.path;
    dest = request_buff + count;
    bcopy(src, dest, strlen(src));
    count += strlen(src);
    if (request_ptr->params != NULL) {
        dest = request_buff + count;
        bcopy("?", dest, 1);
        count++;
    }
    char **pptr;
    int len;
    /* parse params to the path */
    for (pptr = request_ptr->params; request_ptr->params && *pptr != NULL; pptr++) {
        /* params are separated by "&" */
        if (request_buff[count - 1] != '?') {
            bcopy("&", request_buff + count, 1);
            count++;
        }
        src = *pptr;
        dest = request_buff + count;
        len = strlen(src);
        bcopy(src, dest, len);
        count += len;
    }
    bcopy(" ", request_buff + count, 1);
    count++; /* to add a space between path and http_version */
    
    len = strlen(HTTP_VERSION);
    bcopy(HTTP_VERSION, request_buff + count, len);
    count += len;
    
    /* end of the first line */
    bcopy(line_sep, request_buff + count, 2);
    count += 2;
    for (pptr = request_ptr->headers; request_ptr->headers && *pptr != NULL; pptr++) {
        src = *pptr;
        dest = request_buff + count;
        len = strlen(src);
        bcopy(src, dest, len);
        count += len;
        /* headers separated by "/r/n" */
        bcopy(line_sep, request_buff + count, 2);
        count += 2;
    }
    /* end of headers */
    bcopy(line_sep, request_buff + count, 2);
    count += 2;
    
    /* parse data to the request body */
    for (pptr = request_ptr->data; request_ptr->data && *pptr != NULL; pptr++) {
        src = *pptr;
        dest = request_buff + count;
        len = strlen(src);
        bcopy(src, dest, len);
        count += len;
        bcopy(line_sep, request_buff + count, 2);
        count += 2;
    }
    return;
}

void requests(const char *const url, struct request *request_ptr, char *response_buff) {
    char request_buff[MAXLINE];
    struct sockaddr_in servaddr;
    set_http_request(url, request_ptr, request_buff);
    
    struct url s_url;
    parse_url(url, &s_url);
    char *hostname = s_url.host;
    
    int sockfd;
    if ((sockfd = socket(FAMILY, TYPE, 0)) < 0) {
        perror("socket error");
        exit(-1);
    }
    
    set_servaddr(hostname, &servaddr);
    
    /* [connect] included by <sys/socket.h> */
    if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
        perror("connect error");
        exit(-1);
    }
    
    /* [write] and [read] included by <unistd.h> */
    if ((write(sockfd, request_buff, strlen(request_buff))) < 0) {
        perror("write error");
        exit(-1);
    }
    
    int n, count = 0;
    while ((n = read(sockfd, response_buff + count, sizeof(response_buff)))) {
        count += n;
    }
    return;
}

void parse_response(char *response_buff, struct response *response_ptr) {
    
}

int main() {
    struct request s_request;
    bzero(&s_request, sizeof(struct request));
    s_request.method = "GET";
    char *headers[3];
    headers[0] = "Host: www.baidu.com";
    headers[1] = "Connection: close";
    headers[2] = NULL;
    s_request.headers = headers;
    char response_buff[600000];
    requests("http://www.baidu.com/", &s_request, response_buff);
    printf("response_buff is:\n%s\n", response_buff);
    struct node **table = init_table();
    set(table, "key1", "value1");
    set(table, "k2", "v2");
    set(table, "k3", "v3");
    print_table(table);
    set(table, "k3", "v3.1");
    pop(table, "k2");
    print_table(table);
    free(table);
    return 0;
}




