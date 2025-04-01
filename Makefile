# Makefile for Bookshelf Management System

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Regular build target
all: bookshelf

bookshelf: book.c library.c bookshelf.c book.h library.h
	$(CC) $(CFLAGS) book.c library.c bookshelf.c -o bookshelf

# Unity build target
unity: unity_build.c book.h library.h
	$(CC) $(CFLAGS) unity_build.c -o bookshelf_unity

clean:
	rm -f bookshelf bookshelf_unity

.PHONY: all clean unity