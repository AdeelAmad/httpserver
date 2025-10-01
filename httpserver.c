//
// Created by Adeel Ahmad on 1/21/25.
//

#include "socket_handler.h"
#include "io_wrapper.h"
#include "queue.h"
#include "rwlock.h"

#include "responses.h"
#include "regulars.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE (8192 * 2)
#define QUEUE_SIZE  5000
#define MUL 10

regex_t type;
regex_t uri;
regex_t http_str;
regex_t content_length;
regex_t number;
regex_t header;
regex_t request_id;
regex_t value;

queue_t *q;

int threads = 4;

typedef struct request {
    char method[8];
    char uri[64];
    char ver[9];
    char headers[8192];
    char req_id[8192];
} request;

typedef struct file_lock {
    char file[256];
    pthread_rwlock_t lock;
    int users;
} file_lock_t;

pthread_mutex_t lock_table;

file_lock_t *get_file_lock(const char *path, file_lock_t *file_locks) {

    for (int i = 0; i < threads*MUL; i++) {
        if (strcmp(file_locks[i].file, path) == 0) {
            file_locks[i].users++;
            return &file_locks[i];
        }
    }

    for (int i = 0; i < threads*MUL; i++) {
        if (file_locks[i].file[0] == '\0') {
            strncpy(file_locks[i].file, path, sizeof(file_locks[i].file) - 1);
            pthread_rwlock_init(&file_locks[i].lock, 0);
            file_locks[i].users = 1;
            return &file_locks[i];
        }
    }
    return NULL;
}

void release_file_lock(file_lock_t *file_lock) {
    pthread_mutex_lock(&lock_table);

    file_lock->users--;

    if (file_lock->users == 0) {
        file_lock->file[0] = '\0';
        pthread_rwlock_destroy(&file_lock->lock);
    }

    pthread_mutex_unlock(&lock_table);
}

void return_200(int socket) {
    write_bytes(socket, RESPONSE_200, 41);
}

void return_201(int socket) {
    write_bytes(socket, RESPONSE_201, 51);
}

void return_400(int socket) {
    write_bytes(socket, RESPONSE_400, 60);
}

void return_403(int socket) {
    write_bytes(socket, RESPONSE_403, 56);
}

void return_404(int socket) {
    write_bytes(socket, RESPONSE_404, 56);
}

void return_501(int socket) {
    write_bytes(socket, RESPONSE_501, 68);
}

void return_505(int socket) {
    write_bytes(socket, RESPONSE_505, 80);
}

void write_audit(request *r, int status) {
    fprintf(stderr, "%s,%s,%i,%s\n", r->method, r->uri, status, r->req_id);
    fflush(stderr);
}

int version_validation(char version[9], int socket) {
    if (strcmp(version, HTTP_VERSION) == 0) {
        return 0;
    }
    if (strlen(version) > 8) {
        return_400(socket);
        return 400;
    }
    return_505(socket);
    return 505;
}

int get(char *path, int socket) {

    int fd = open(path + sizeof(char), O_RDONLY);
    char c_len[BUFFER_SIZE] = { 0 };

    struct stat statbuf;
    stat(path + sizeof(char), &statbuf);

    if (fd < 0 || S_ISDIR(statbuf.st_mode)) {
        //        logic for 403 404
        if (errno == EACCES || S_ISDIR(statbuf.st_mode)) {
            return_403(socket);
            close(fd);
            return 403;
        }

        return_404(socket);
        close(fd);
        return 404;
    }

    sprintf(c_len, "HTTP/1.1 200 OK\r\nContent-Length: %i\r\n\r\n", (int) statbuf.st_size);
    write_bytes(socket, c_len, (int) strlen(c_len));

    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;
    ssize_t bytes_write = 0;

    char get_buffer[BUFFER_SIZE] = { 0 };

    while ((bytes_read = read(fd, get_buffer, BUFFER_SIZE)) > 0) {
        bytes_written = 0;
        while (bytes_written < bytes_read) {
            bytes_write = write(socket, get_buffer + bytes_written, bytes_read - bytes_written);
            bytes_written += bytes_write;
        }
    }
    close(fd);
    return 200;
}

int put(char *path, int socket, int content_len) {
    struct stat statbuf;
    stat(path + sizeof(char), &statbuf);

    if (S_ISDIR(statbuf.st_mode)) {
        return_403(socket);
        return 403;
    }

    int create_flag = 1;
    int fd = open(path + sizeof(char), O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644);
    if (fd < 0) {
        create_flag = 0;
        close(fd);
        fd = open(path + sizeof(char), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    }

    if (fd < 0) {
        return_403(socket);
        close(fd);
        return 403;
    }

    ssize_t bytes_read;
    ssize_t total_bytes_read = 0;
    ssize_t bytes_written;
    ssize_t bytes_write;

    char put_buffer[BUFFER_SIZE] = { 0 };



    while ((total_bytes_read < content_len)
           && ((bytes_read = read(socket, put_buffer, BUFFER_SIZE)) > 0)) {
        bytes_written = 0;
        while (bytes_written < bytes_read) {
            bytes_write = write(fd, put_buffer + bytes_written, bytes_read - bytes_written);
            bytes_written += bytes_write;
        }
        total_bytes_read += bytes_read;
    }

    fsync(fd);

    if (create_flag) {
        return_201(socket);
        close(fd);
        return 201;
    } else {
        return_200(socket);
        close(fd);
        return 200;
    }
}

void *handle_request(void *arg) {
    file_lock_t *table = (file_lock_t *) arg;
    int *sock;
    while (1) {
        queue_pop(q, (void **) &sock);
        {
            int socket = *sock;
            int response = 0;

            request *req = calloc(1, sizeof(request));
            char buff[8192] = { 0 };
            read_bytes(socket, buff, 81, 1);
            regmatch_t meth[1];
            char content_len[8192] = { 0 };
            char req_id[8192] = { 0 };
            char c_len[8192] = { 0 };

            //    Parse request line
            regexec(&type, buff, 1, meth, 0);
            strncpy(req->method, buff + (meth[0].rm_so * sizeof(char)),
                meth[0].rm_eo - meth[0].rm_so - 1);
            regexec(&uri, buff, 1, meth, 0);
            strncpy(req->uri, buff + (meth[0].rm_so * sizeof(char)), meth[0].rm_eo - meth[0].rm_so);
            regexec(&http_str, buff, 1, meth, 0);
            strncpy(req->ver, buff + (meth[0].rm_so * sizeof(char)), meth[0].rm_eo - meth[0].rm_so);

        read_header:
            memset(buff, 0, sizeof(buff));
            read_bytes(socket, buff, 8192, 1);
            if ((strcmp("\r\n", buff) == 0)) {
                memset(buff, 0, sizeof(buff));
            } else {
                if (strcmp("", buff) == 0) {
                    return_400(socket);
                    close_connection(socket);
                    write_audit(req, 400);
                    free(req);
                    free(sock);
                    continue;
                }
                sprintf(req->headers, "%s%s", req->headers, buff);
                goto read_header;
            }

            char *head = strtok(req->headers, "\r\n");

        validate_header:
            if (head != NULL) {
                if (regexec(&header, head, 0, NULL, 0)) {
                    return_400(socket);
                    close_connection(socket);
                    write_audit(req, 400);
                    free(req);
                    free(sock);

                    continue;
                }
                if (regexec(&content_length, head, 1, meth, 0) == 0) {
                    strncpy(content_len, head + (meth[0].rm_so * sizeof(char)),
                        meth[0].rm_eo - meth[0].rm_so);
                }
                if (regexec(&request_id, head, 1, meth, 0) == 0) {
                    strncpy(req_id, head + (meth[0].rm_so * sizeof(char)),
                        meth[0].rm_eo - meth[0].rm_so);
                }
                head = strtok(NULL, "\r\n");
                goto validate_header;
            }

            if (regexec(&request_id, req_id, 0, NULL, 0) == 1) {
                req->req_id[0] = '0';
                req->req_id[1] = '\0';
            } else {
                regexec(&value, req_id, 1, meth, 0);
                strncpy(req->req_id, req_id + (meth[0].rm_so * sizeof(char)) + 1,
                    meth[0].rm_eo - meth[0].rm_so);
            }

            if ((response = version_validation(req->ver, socket)) > 0) {
                close_connection(socket);
                write_audit(req, response);
                free(req);
                free(sock);
                continue;
            }

            if (strcmp(req->method, "GET") == 0) {
                pthread_mutex_lock(&lock_table);
                file_lock_t *fl = get_file_lock(req->uri, table);
                pthread_mutex_unlock(&lock_table);

                pthread_rwlock_rdlock(&fl->lock);
                response = get(req->uri, socket);
                write_audit(req, response);
                pthread_rwlock_unlock(&fl->lock);

                release_file_lock(fl);

                close_connection(socket);
                free(req);
                free(sock);
                continue;
            }
            if (strcmp(req->method, "PUT") == 0) {
                if (regexec(&content_length, content_len, 0, NULL, 0) == 1) {
                    return_400(socket);
                    close_connection(socket);
                    write_audit(req, 400);
                    free(sock);
                    continue;
                }
                regexec(&number, content_len, 1, meth, 0);
                strncpy(c_len, content_len + (meth[0].rm_so * sizeof(char)),
                    meth[0].rm_eo - meth[0].rm_so);

                pthread_mutex_lock(&lock_table);
                file_lock_t *fl = get_file_lock(req->uri, table);
                pthread_mutex_unlock(&lock_table);

                pthread_rwlock_wrlock(&fl->lock);
                response = put(req->uri, socket, (int) strtol(c_len, NULL, 10));
                write_audit(req, response);
                pthread_rwlock_unlock(&fl->lock);

                release_file_lock(fl);

                close_connection(socket);
                free(req);
                free(sock);
                continue;
            }

            if ((strcmp(req->method, "") == 0)) {
                return_400(socket);
                close_connection(socket);
                write_audit(req, 400);
                free(req);
                free(sock);
                continue;
            } else {
                return_501(socket);
                close_connection(socket);
                write_audit(req, 501);
                free(req);
                free(sock);
                continue;
            }
        }

    }
    return NULL;
}

int main(int argc, char *args[]) {
    int opt;

    while ((opt = getopt(argc, args, "t:")) >= 0) {
        switch (opt) {
        case 't':
            threads = strtol(optarg, NULL, 10);
            if (threads <= 0) {
                fprintf(stderr, "invalid threads %s", optarg);
                return 1;
            }
            break;
        default: fprintf(stderr, "Usage: %s [-t threads] <port>\n", args[0]); return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    int port = (int) strtol(args[optind], NULL, 10);

    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    printf("port: %i threads: %i\n", port, threads);

    struct listener_sock *l_sock = listen_port(port);

    if (l_sock == NULL) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    regcomp(&type, TYPE_REGEX, REG_EXTENDED);
    regcomp(&uri, URI_REGEX, REG_EXTENDED);
    regcomp(&http_str, HTTP_REGEX, REG_EXTENDED);
    regcomp(&content_length, CONT_LEN, REG_EXTENDED | REG_NEWLINE);
    regcomp(&number, NUMBER, REG_EXTENDED);
    regcomp(&header, VALID_HEADER, REG_EXTENDED);
    regcomp(&request_id, REQ_ID, REG_EXTENDED);
    regcomp(&value, VALUE, REG_EXTENDED);

    q = queue_new(QUEUE_SIZE);

    pthread_mutex_init(&lock_table, 0);
    file_lock_t file_locks[threads*MUL];

    for (int i = 0; i < threads*MUL; i++) {
        file_locks[i].file[0] = '\0';
        file_locks[i].users = 0;
    }

    for (int i = 0; i < threads; i++) {
        pthread_create(&(pthread_t) { 0 }, NULL, handle_request, file_locks);
    }

    while (1) {
        int *sock = calloc(1, sizeof(int));
        *sock = accept_connection(l_sock);
        if (*sock == -1) {
            close_connection(*sock);
            free(sock);
            continue;
        }
        queue_push(q, sock);
    }

    close_port(l_sock);
    queue_delete(&q);
    return 0;
}
