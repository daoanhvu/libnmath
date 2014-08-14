#include <stdlib.h>
#include <stdio.h>
#include "fp128.h"

struct tagTest {
	FP128 value;
};

int main(int argc, char* args[]) {
	FILE *f;
	int len;
	int chCode, temp;
	unsigned long i, size = 0;
	char buffer[64];

	f = fopen("/cygdrive/d/data.dat", "rb");

	if(f != NULL) {
		fseek (f, 0L, SEEK_END);
		size = ftell(f);

		printf("File size: %ld byte(s)\n", size);
		fseek (f, 0L, SEEK_SET);
		len = fread(buffer, 1, size, f);	
		fclose(f);

		printf("\n");
		for (i=0; i<len; i++) {
			printf("%X ", buffer[i]);
		}

		printf("\n");
		i = 0;
		while(i < len) {

			if(buffer[i] > 0){
				printf("(%d)%c ", i, buffer[i]);
				i++;
			} else if( (buffer[i] & 0xE0) == 0xE0) {
				chCode = buffer[i] & 0x0000000F;
				chCode = (( chCode  << 12) | ((buffer[i+1] & 0x0000003F) << 6)) | (buffer[i+2] & 0x0000003F);
				printf("(%d)%X ", i, chCode);
				i += 3;
			} else if( (buffer[i] & 0xC0) == 0xC0) {
				chCode = buffer[i];
				chCode = ( (chCode & 0x001f) << 6) | (buffer[i+1] & 0x3f);
				printf("(%d)%X ", i, chCode);
				i += 2;
			}else{
				//printf("(%d)%X ", i, buffer[i]);
				i++;
			}
			
		}
		printf("\n");
	}
	return 0;
}