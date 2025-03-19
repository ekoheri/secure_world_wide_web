#ifndef HTTP_H
#define HTTP_H

typedef struct {
    char method[16];
    char *directory;
    char *uri;
    char http_version[16];
    char *query_string;
    char *path_info;
    char *body_data;
    char *request_time;
    int content_length;
} RequestHeader;

typedef struct {
    const char *http_version;
    int status_code;
    const char *status_message;
    const char *mime_type;
    unsigned long content_length;
} ResponseHeader;

RequestHeader parse_request_line(char *request);

char *handle_method(int *response_size, RequestHeader req_header);
#endif