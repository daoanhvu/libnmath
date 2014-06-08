#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "common.h"
#include "nlablexer.h"

int main(int argc, char *agr[]){
	int i;
	TokenList tokenList;
	tokenList.size = 0;
	tokenList.loggedSize = 0;
	tokenList.list = NULL;

	parseTokens("x^2 + 6 - sin(x)", 16, &tokenList);

	if(getError()>=0){
		for(i = 0; i < tokenList.size; i++)
			free(tokenList.list[i]);
		return 1;
	}

	printf("Number of token: %d", tokenList.size );

	//Release tokenList
	for(i = 0; i < tokenList.size; i++)
		free(tokenList.list[i]);

	return 0;
}
