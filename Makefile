################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Yanbo Li,                          								   #
#          Jingxia Pang                                      				   #
#                                                                              #
################################################################################

CC=gcc
CFLAGS=-I.
FLAGS=-Wall -Werror -g
OBJ = log.o echo_server.o y.tab.o lex.yy.o parse.o
DEPS = log.h parse.h y.tab.h

default: all

all: echo_server echo_client

lex.yy.c: lexer.l
	flex $^

y.tab.c: parser.y
	yacc -d $^

%.o: %.c $(DEPS)
	# $(CC) -c -o $@ $< $(CFLAGS)
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS)

echo_server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

echo_client:
	$(CC) echo_client.c -o echo_client -Wall -Werror

clean:
    # rm -rf *.o
	rm -f *~ *.o echo_server echo_client server lex.yy.c y.tab.c y.tab.h


