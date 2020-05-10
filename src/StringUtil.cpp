#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <tchar.h>
#endif
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

void nmath::getOperatorChar(int operatorType, char *opCh) {
	switch (operatorType){
	case PLUS:
		(*opCh) = '+';
		return;
	case MINUS:
		(*opCh) = '-';
		return;
	case MULTIPLY:
		(*opCh) = '*';
		return;
	case DIVIDE:
		(*opCh) = '/';
		return;
	case POWER:
		(*opCh) = '^';
		return;
	default:
		(*opCh) = 0;
		return;
	}
}

void nmath::toString(const NMAST *t, char *str, int *curpos, int len) {
	double fr;
	long lval;
	int i, l;
	char operatorChar = 0;
	char tmp[16];

	if (t == NULL)
		return;

	switch (t->type){
	case NUMBER:
		lval = l_cast(t->value, &fr);
		if (fr != 0.0){
			l = sprintf(tmp, "%lf", t->value);
		}
		else{
			l = sprintf(tmp, "%ld", lval);
		}

		for (i = 0; i<l; i++) {
			str[(*curpos)] = tmp[i];
			(*curpos)++;
		}
		break;

	case PI_TYPE:
		str[(*curpos)] = 'P';
		str[(*curpos) + 1] = 'I';
		(*curpos) += 2;
		break;

	case E_TYPE:
		str[(*curpos)] = 'e';
		(*curpos)++;
		break;

	case VARIABLE:
	case NAME:
		str[(*curpos)] = t->variable;
		(*curpos)++;
		break;

	case PLUS:
	case MINUS:
	case MULTIPLY:
	case DIVIDE:
	case POWER:
		getOperatorChar(t->type, &operatorChar);

		if ((t->parent != NULL) && ((t->parent)->priority < t->priority)){
			str[(*curpos)] = '(';
			(*curpos)++;
		}


		if (t->left != NULL)
			toString(t->left, str, curpos, len);

		str[(*curpos)] = operatorChar;
		(*curpos)++;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		if ((t->parent != NULL) && ((t->parent)->priority < t->priority)) {
			str[(*curpos)] = ')';
			(*curpos)++;
		}
		break;

	case SIN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';

		//printf("sin(");
		str[(*curpos)] = 's';
		str[(*curpos) + 1] = 'i';
		str[(*curpos) + 2] = 'n';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case ASIN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("asin(");
		str[(*curpos)] = 'a';
		str[(*curpos) + 1] = 's';
		str[(*curpos) + 2] = 'i';
		str[(*curpos) + 3] = 'n';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case COS:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("cos(");
		str[(*curpos)] = 'c';
		str[(*curpos) + 1] = 'o';
		str[(*curpos) + 2] = 's';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case ACOS:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("acos(");
		str[(*curpos)] = 'a';
		str[(*curpos) + 1] = 'c';
		str[(*curpos) + 2] = 'o';
		str[(*curpos) + 3] = 's';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case ATAN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("atan(");
		str[(*curpos)] = 'a';
		str[(*curpos) + 1] = 't';
		str[(*curpos) + 2] = 'a';
		str[(*curpos) + 3] = 'n';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case LN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("ln(");
		str[(*curpos)] = 'l';
		str[(*curpos) + 1] = 'n';
		str[(*curpos) + 2] = '(';
		(*curpos) += 3;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case LOG:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("log(");
		str[(*curpos)] = 'l';
		str[(*curpos) + 1] = 'o';
		str[(*curpos) + 2] = 'g';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;
		if (t->left != NULL)
			toString(t->left, str, curpos, len);
		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case SQRT:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("sqrt(");
		str[(*curpos)] = 's';
		str[(*curpos) + 1] = 'q';
		str[(*curpos) + 2] = 'r';
		str[(*curpos) + 3] = 't';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;
		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case SEC:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("sec(");
		str[(*curpos)] = 's';
		str[(*curpos) + 1] = 'e';
		str[(*curpos) + 2] = 'c';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;
		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case AND:
		str[(*curpos)] = 'A';
		str[(*curpos) + 1] = 'N';
		str[(*curpos) + 2] = 'D';
		(*curpos) += 3;
		break;

	case OR:
		str[(*curpos)] = 'O';
		str[(*curpos) + 1] = 'R';
		(*curpos) += 2;
		break;
	}
}
