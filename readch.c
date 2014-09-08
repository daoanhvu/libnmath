#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* args[]) {
	FILE *file;
	char str[64];
	int len, size, i;

	file = fopen(args[1], "rb");
	if( file != NULL ) {
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		len = fread(str, 1, size, file);
		fclose(file);

		printf("Number of bytes read: %d\n", len);
		for(i = 0; i<len; i++) {
			printf("0x%X(%d) ", str[i], str[i]);
		}
		printf("\n");
	}
}