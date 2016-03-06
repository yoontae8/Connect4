
CC = arm-linux-gcc

PWD = $(shell pwd)

TARGET = connect4
SRCS = main

LIBDIR = ./mylib

all: serv_pgm clnt_pgm

serv_pgm:
	$(CC) $(SRCS)_serv.c -o $(TARGET)_serv -L$(LIBDIR) -I$(LIBDIR) -lpthread -lm -llogic -lts -limg
clnt_pgm:
	$(CC) $(SRCS)_clnt.c -o $(TARGET)_clnt -L$(LIBDIR) -I$(LIBDIR) -lpthread -lm -llogic -lts -limg

clean:
	rm -rf $(TARGET)_*
