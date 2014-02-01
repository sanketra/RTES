#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char **argv) {
	FILE *fp;
	char *line = NULL;
	size_t n = 0;
	size_t len;

	fp = fopen(argv[1], "r");

	if (fp == NULL)
    		return 1;

	while ((len = getline(&line, &n, fp)) != -1) {
    		printf("Retrieved line of length %zd:\n", len);
    		printf("%s\n", line);
		char *pch = strtok(line, " ");
  		
		while (pch != NULL) {
    			printf ("%s\n", pch);
    			pch = strtok(NULL, " ");
 		}
	}

	if (line)
    		free(line);
	return 0;
}
