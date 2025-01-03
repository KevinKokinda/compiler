CC = gcc
CFLAGS = -Wall -Wextra -I./include
SRCS = src/lexer.c src/parser.c src/ast.c src/symbol_table.c src/optimizer.c src/codegen.c src/main.c
OBJS = $(SRCS:.c=.o)
TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./test.sh

clean:
	rm -f $(OBJS) $(TARGET) output/*.asm tests/*.o tests/*.exe

.PHONY: all test clean