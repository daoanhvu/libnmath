#include "StringUtil.h"

using namespace nmath;

/*
return the UTF-8 character code at index of the string
*/
int nmath::getCharacter(const char *str, int length, int index, int *nextIdx, int *errorCode) {

	int result = str[index] & 0x000000FF;

	/*
	if((inStr[idx] & 0x80) != 0x80) {
	*nextIdx = index + 1;
	return result;
	}
	*/

	*errorCode = NMATH_NO_ERROR;
	if ((str[index] & 0xF8) == 0xF0) {
		/* 11110XXX We need to read three bytes more */
		result = str[index] & 0x00000007;
		result = (((result << 18) | ((str[index + 1] & 0x0000003F) << 12)) | ((str[index + 2] & 0x0000003F) << 6)) | (str[index + 3] & 0x0000003F);
		*nextIdx = index + 4;
	}
	else if ((str[index] & 0xF0) == 0xE0) {
		/* 1110XXXX We need to read two bytes more */
		result = str[index] & 0x0000000F;
		result = ((result << 12) | ((str[index + 1] & 0x0000003F) << 6)) | (str[index + 2] & 0x0000003F);
		*nextIdx = index + 3;
	}
	else if ((str[index] & 0xE0) == 0xC0) {
		/* 110XXXXX We need to read one byte more */
		result = str[index] & 0x0000001F;
		result = (result << 6) | (str[index + 1] & 0x0000003f);
		*nextIdx = index + 2;
	}
	else {
		*errorCode = ERROR_MALFORMED_ENCODING;
	}

	return result;
}

int nmath::isDigit(char c) {
	if (c >= 48 && c <= 57)
		return TRUE;
	return FALSE;
}

int nmath::isInArray(char *vars, char c) {
	if (c == vars[0])
		return 0;

	if (c == vars[1])
		return 1;

	if (c == vars[2])
		return 2;

	return -1;
}