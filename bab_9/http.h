#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE 1024
// #define FOLDER_DOCUMENT "/home/eko/socket_programming/bab_7/"
// #define CGI_PATH "cgi/cgi"

typedef struct {
    char method[256];
    char uri[BUFFER_SIZE];
    char http_version[256];
    char query_string[BUFFER_SIZE];
    char post_data[BUFFER_SIZE];
} RequestHeader;

typedef struct {
    const char *http_version;
    int status_code;
    const char *status_message;
    const char *mime_type;
} ResponseHeader;
RequestHeader parse_request_line(char *request);

char *handle_method(int *response_size, RequestHeader req_header);
#endif