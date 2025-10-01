//
// Created by Adeel Ahmad on 2/24/25.
//

#ifndef AAHMAD9_REGULARS_H
#define AAHMAD9_REGULARS_H

#define TYPE_REGEX   "^[a-zA-Z]{1,8} "
#define URI_REGEX    "/[a-zA-Z0-9.-]{0,63}"
#define HTTP_REGEX   "HTTP/[0-9].[0-9]+"
#define HTTP_VERSION "HTTP/1.1"
#define CONT_LEN     "Content-Length: [0-9]+"
#define REQ_ID       "Request-Id: .+"

#define NUMBER       "[0-9]+"
#define VALUE        " .*$"
#define VALID_HEADER "[a-zA-Z0-9.-]{1,128}: [ -~]{1,128}"

#endif //AAHMAD9_REGULARS_H
