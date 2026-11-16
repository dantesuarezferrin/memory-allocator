CC = gcc
CFLAGS = -std=c11 -D_DEFAULT_SOURCE -Wall -Wextra -Werror -pedantic -g -fsanitize=undefined -Iinclude
LDFLAGS = -fsanitize=undefined

SRC = src/memalloc.c tests/test_main.c
OBJ = $(SRC:.c=.o)
TARGET = test_runner

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run clean
