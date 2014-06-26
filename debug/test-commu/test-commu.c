#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdlib.h>

int main(int argc, const char* argv[]){
	FILE* pfile;
	char command[30];
	strcpy(command, "../../node -p");
	strcat(command, argv[1]);
	printf("excute %s\n", command);
	pfile = popen(command, "r");
	if (pfile == NULL){
		printf("popen failed\n");
		exit(1);
	}
	char c;
	while(!feof(pfile)){
		c = getc(pfile);
		printf("%c", c);
	}
	return 0;
}
