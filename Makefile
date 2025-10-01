CC = clang
CFLAGS=-Wall -Wextra -Werror -pedantic
EXEC = httpserver
HEADERS = adeel_socket_handler.h adeel_io_wrapper.h queue.h responses.h regulars.h rwlock.h

all: $(EXEC)

$(EXEC): $(EXEC).o adeel_socket_handler.o adeel_io_wrapper.o queue.o rwlock.o
	$(CC) $^ $(LFLAGS) -o $@
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<
format:
	clang-format -i -style=file *.[ch]
clean:
	rm -rf $(EXEC) *.o

.PHONY: all clean format
