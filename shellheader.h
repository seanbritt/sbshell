#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>

#define BI_LEN 7
#define RED_LEN 5
#define POSSIBLE_BUILTIN 1
#define IMPOSSIBLE_BUILTIN 0

int cd(int argc, char **argv);
void clr();
int dir(int argc, char **argv);
void environ(char **envp);
void echo(char **argv);
void help();
void _pause();
void quit();
