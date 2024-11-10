#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 8192

void handle_url(char *url, char *hostname, char *path, int *port, char *protocol);
char *handle_respose(char *url, char *form_data);

#endif
