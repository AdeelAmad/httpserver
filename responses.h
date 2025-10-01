//
// Created by Adeel Ahmad on 2/24/25.
//

#ifndef AAHMAD9_RESPONSES_H
#define AAHMAD9_RESPONSES_H

#define RESPONSE_200 "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n"
#define RESPONSE_201 "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n"

#define RESPONSE_400 "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n"
#define RESPONSE_403 "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n"
#define RESPONSE_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n"

#define RESPONSE_500                                                                               \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server Error\n"
#define RESPONSE_501 "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n"
#define RESPONSE_505                                                                               \
    "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not Supported\n"

#endif //AAHMAD9_RESPONSES_H
