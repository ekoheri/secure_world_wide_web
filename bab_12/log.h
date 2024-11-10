#ifndef LOG_H
#define LOG_H

#define LOG_DIR "/home/eko/socket_programming/bab_12/logs/"

void create_log_directory();
void write_log(const char *format, ...);

#endif // LOG_H
