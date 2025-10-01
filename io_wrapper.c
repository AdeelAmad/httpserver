//
// Created by Adeel Ahmad on 1/22/25.
//

#include "io_wrapper.h"

ssize_t read_bytes(int file_stream, char buffer[], int n, int escape) {

    int carriage = 0;
    int ret = 0;

    ssize_t bytes_read = 0;

    while (
        (bytes_read < n) && !(carriage && ret) && (read(file_stream, buffer + bytes_read, 1) > 0)) {
        if (escape) {
            if (carriage && buffer[bytes_read] == '\n') {
                ret = 1;
            } else {
                carriage = 0;
                ret = 0;
            }
            if (buffer[bytes_read] == '\r') {
                carriage = 1;
            }
        }
        bytes_read++;
    }

    return bytes_read;
}

ssize_t write_bytes(int file_stream, char buffer[], int n) {
    ssize_t bytes_written = 0;

    while (bytes_written < n) {
        ssize_t bytes_write = write(file_stream, buffer + bytes_written, n - bytes_written);
        bytes_written += bytes_write;
    }

    return bytes_written;
}
