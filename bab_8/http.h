#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE 1024
//Folder dokumen HTML mengarah ke dokumen/
#define FOLDER_DOCUMENT "dokumen/"
#define RESOURCE_DEFAULT "index.html"

typedef struct {
    char method[BUFFER_SIZE];
    char uri[BUFFER_SIZE];
    char http_version[BUFFER_SIZE];
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