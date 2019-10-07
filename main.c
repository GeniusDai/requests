//
//  main.c
//  requests
//
//  Created by hao on 2019/10/7.
//  Copyright © 2019 hao. All rights reserved.
//

#include "main.h"

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
