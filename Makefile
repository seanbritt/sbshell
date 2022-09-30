
# Sean Britt.  Makefile for myShell program

CC = gcc
FLAG = -I.
DEP = shellheader.h
OFILES = myshell.c builtins.c
OPTS = -Wall -Werror

myShell: $(OFILES)
	$(CC) $(OPTS) -o $@ $^ $(FLAG)

%.o: %.c $(DEP)
	$(CC) $(OPTS) -c -o $@ $< $(FLAG)
clean:
	rm -fr \#*\#  *.o *~ *.out myShell

