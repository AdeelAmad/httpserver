//
// Created by Adeel Ahmad on 1/22/25.
//

#ifndef AAHMAD9_ADEEL_IO_WRAPPER_H
#define AAHMAD9_ADEEL_IO_WRAPPER_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

ssize_t read_bytes(int file_stream, char buffer[], int n, int escape);

ssize_t write_bytes(int file_stream, char buffer[], int n);

#endif //AAHMAD9_ADEEL_IO_WRAPPER_H
