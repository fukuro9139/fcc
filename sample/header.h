#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

char *receive_input(char *str){
	return fgets(str, 128, stdin);
}