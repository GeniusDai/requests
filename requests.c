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

/**
 * response_ptr->content: memory locates in the response_buff.
 * the other fields in the struct locate in the heap, that is acheived by using malloc.
 */
void parse_response(char *response_buff, struct response *response_ptr) {
    int len = strlen(response_buff);
    int i;
    for (i = 0; i < len; i++) {
        if (strncmp(response_buff + i, "\r\n\r\n", 4) == 0) {
            break;
        }
    }
    
    response_ptr->content = response_buff + i + 4;
    
    char response_header[i + 2];
    bcopy(response_buff, response_header, i + 2);
    
    int count = 0;
    for (i = 0; i < sizeof(response_header); i++) {
        if (strncmp(response_header + i, "\r\n", 2) == 0) {
            count++;
        }
    }
    char *line;
    char *key;
    char *value;
    char *elem;
    struct node **table = init_table();
    char *headers[count];
    line = strtok(response_header, "\r\n");
    for (i = 0; i < count; i++) {
        headers[i] = line;
        line = strtok(NULL, "\r\n");
    }
    
    /** parse first line */
    strtok(headers[0], " ");
    response_ptr->status_code = atoi(strtok(NULL, " "));
    elem = strtok(NULL, " ");
    response_ptr->reason = malloc(sizeof(char) * (strlen(elem) + 1));
    bcopy(elem, response_ptr->reason, strlen(elem));
    /** NULL terminates */
    bcopy("\0", response_ptr->reason + 2, 1);
    
    /** parse headers */
    for (i = 1; i < count; i++) {
        int j, len;
        len = strlen(headers[i]);
        for (j = 0; j < len; j++) {
            if (strncmp(headers[i] + j, ": ", 2) == 0)
                break;
        }
        key = malloc(sizeof(char) * (j + 1));
        value = malloc(sizeof(char) * (len - j - 2 + 1));
        bcopy(headers[i], key, j);
        bcopy("\0", key + j, 1);
        bcopy(headers[i] + j + 2, value, len - j - 2);
        bcopy("\0", value + len - j - 2, 1);
        set(table, key, value);
    }
    response_ptr->headers = table;
    return;
}

int main() {
    struct request s_request;
    bzero(&s_request, sizeof(struct request));
    s_request.method = "GET";
    char *headers[2];
    headers[0] = "Connection: close";
    headers[1] = NULL;
    s_request.headers = headers;
    char response_buff[600000];
    requests("http://www.baidu.com/", &s_request, response_buff);
    struct response s_response;
    parse_response(response_buff, &s_response);
    
    printf("status_code: %d\n", s_response.status_code);
    printf("\n");
    printf("reason: %s\n", s_response.reason);
    printf("\n");
    printf("headers:\n");
    print_table(s_response.headers);
    printf("\n");
    printf("content:\n%s", s_response.content);
    return 0;
}
