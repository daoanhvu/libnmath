#include <cstdlib>
#include <cstring>
#include "utf8.h"
#include "common_data.h"

int readNext(const char* str, int len, short *out) {
	int i = 0;

	while(i<len) {

	}

	return 0;
}

/*
        return the UTF-8 character code at index of the string
    */
int getCharacter(const char *str, int length, int index, int *nextIdx, int *errorCode) {
	if (nextIdx != NULL) {
		*nextIdx = index + 1;
	}
	if (errorCode != NULL) {
		*errorCode = NMATH_NO_ERROR;
	}
	if (str == NULL || nextIdx == NULL || errorCode == NULL) {
		return 0;
	}
	if (index < 0 || index >= length) {
		*errorCode = ERROR_MALFORMED_ENCODING;
		return 0;
	}

	unsigned char b0 = (unsigned char)str[index];
	int result = b0 & 0x000000FF;
	int extra = 0;

	/* ASCII, one-byte character */
	if ((b0 & 0x80) == 0) {
		*nextIdx = index + 1;
		return result;
	}

	if ((b0 & 0xF8) == 0xF0) {
		/* 11110XXX We need to read three bytes more */
		result = b0 & 0x00000007;
		extra = 3;
	}
	else if ((b0 & 0xF0) == 0xE0) {
		/* 1110XXXX We need to read two bytes more */
		result = b0 & 0x0000000F;
		extra = 2;
	}
	else if ((b0 & 0xE0) == 0xC0) {
		/* 110XXXXX We need to read one byte more */
		result = b0 & 0x0000001F;
		extra = 1;
	}
	else {
		*errorCode = ERROR_MALFORMED_ENCODING;
		*nextIdx = index + 1;
		return 0;
	}

	if (index + extra >= length) {
		*errorCode = ERROR_MALFORMED_ENCODING;
		*nextIdx = index + 1;
		return 0;
	}

	for (int i = 1; i <= extra; i++) {
		unsigned char bx = (unsigned char)str[index + i];
		if ((bx & 0xC0) != 0x80) {
			*errorCode = ERROR_MALFORMED_ENCODING;
			*nextIdx = index + 1;
			return 0;
		}
		result = (result << 6) | (bx & 0x3F);
	}

	*nextIdx = index + extra + 1;
	return result;
}
