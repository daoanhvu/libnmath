#include <stdlib.h>
#include <stdio.h>
#include <windef.h>
//#include <GL/glew.h>
#include <GL/freeglut.h>

#include <common.h>
#include <nlabparser2.h>
#include <nmath.h>

void render();
int initVerticeFromFunction();
void printError(int col, int code);

float* vertices = NULL;
int* rowInfo = NULL;

int main(int argc, char* args[]){
	/*
	glutInit(&argc, args);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1000, 700);
	glutInitWindowPosition(30, 30);
	
	glutCreateWindow("Graph Plotter");
	
	glutDisplayFunc(render);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glutMainLoop();
	*/
	initVerticeFromFunction();
	return 0;
}

void render(){
}

int initVerticeFromFunction() {
	char strF[]="f(x,y)=sin(x) + cos(y)";
	int len = 10;
	TokenList lst;
	int i, ret=0;
	double z;
	double param[2];
	Function *f;

	lst.loggedSize = 16;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;
	
	len = strlen(strF);
	/* build the tokens list from the input string */
	parseTokens(strF, len, &lst);

	if(getErrorCode() != NO_ERROR){
		printError(getErrorColumn(), getErrorCode());
		for(i = 0; i<lst.size; i++)
			free(lst.list[i]);
		free(lst.list);
		return ERROR_LEXER;
	}

	f = parseFunctionExpression(&lst);
	
	//Release token list
	for(i = 0; i<lst.size; i++)
		free(lst.list[i]);
	free(lst.list);
	
	if(getErrorCode() != NO_ERROR){
		printError(getErrorColumn(), getErrorCode());
		if(f != NULL)
			releaseFunct(f);
		return ERROR_PARSE;
	}
	printf("\n");
	param[0] = -2.0;
	while(param[0] <= 2.0f){
		param[1] = -2.0;
		while(param[1] <= 2.0f){
			z = calc(f, param, 2, &ret);
			printf( "f(%lf,%lf) = %lf\n", param[0], param[1], z );
			param[1] += 0.2;
		}
		param[0] += 0.2;
	}
	printf("\n");
	
	if(f != NULL)
		releaseFunct(f);
	
	return NO_ERROR;
}

void printError(int col, int code){
	switch(code){
		case ERROR_DIV_BY_ZERO:
			break;

		case ERROR_LOG:
			break;

		case ERROR_OPERAND_MISSING:
			break;

		case ERROR_PARSE:
			break;

		case ERROR_TOO_MANY_FLOATING_POINT:
			printf("Too many floating point at %d\n", col);
			break;

		case ERROR_PARENTHESE_MISSING:
			printf("Missing parenthese at %d\n", col);
			break;
			
		case ERROR_TOO_MANY_PARENTHESE:
			printf("Too many parenthese at %d\n", col);
			break;

		case ERROR_OUT_OF_DOMAIN:
			break;

		case ERROR_SYNTAX:
			break;

		case ERROR_NOT_AN_EXPRESSION:
			printf("Bad expression found at %d\n", col);
			break;

		case ERROR_BAD_TOKEN:
			printf("A bad token found at %d\n", col);
			break;

		case ERROR_LEXER:
			break;

		case ERROR_PARSING_NUMBER:
			break;
	}
}