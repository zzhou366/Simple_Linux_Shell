C = gcc
OPTION = -o -g -Wall -Werror

all: 
	$(C) $(OPTION) storage.c mysh.c -o mysh
	$(C) $(OPTION) main.c -o main




