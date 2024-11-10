#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE 1024
#define FOLDER_DOCUMENT "/home/eko/socket_programming/bab_7/"
#define CGI_PATH "/home/eko/socket_programming/bab_9/cgi/cgi"

typedef struct {
    char method[BUFFER_SIZE];
    char uri[BUFFER_SIZE];
    char http_version[BUFFER_SIZE];
    char query_string[BUFFER_SIZE];
    char post_data[BUFFER_SIZE];
} RequestHeader;

typedef struct {
    const char *http_version;
    int status_code;
    const char *status_message;
    const char *mime_type;
    unsigned long content_length;
    const char *encrypted;
    const char *public_key;
} ResponseHeader;

RequestHeader parse_request_line(char *request);

char *handle_method(int *response_size, RequestHeader req_header);
#endif