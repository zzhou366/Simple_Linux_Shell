C = gcc
OPTION = -o -g -Wall -Werror

all: 
	$(C) $(OPTION) storage.c mysh.c -o mysh
	




