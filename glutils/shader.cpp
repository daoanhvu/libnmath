#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.h"

using namespace std;

GLuint loadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	} else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()) {
		std::string Line = "";
		while(getline(FragmentShaderStream, Line)) {
			FragmentShaderCode += "\n" + Line;
		}
		FragmentShaderStream.close();
	}

	GLint actionResult = GL_FALSE;
	int infoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(vertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &actionResult);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (actionResult != GL_TRUE) {
		if ( infoLogLength > 0 ){
			std::vector<char> VertexShaderErrorMessage(infoLogLength+1);
			glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}
		glDeleteShader(vertexShaderID);
		return 0;
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(fragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(fragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &actionResult);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (actionResult != GL_TRUE) {
		if ( infoLogLength > 0 ){
			std::vector<char> FragmentShaderErrorMessage(infoLogLength+1);
			glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			printf("%s\n", &FragmentShaderErrorMessage[0]);
		}
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
		return 0;
	}

	// Link the program
	printf("Linking program started...\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);
	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &actionResult);
	if (actionResult != GL_TRUE) {
		std::cout << "Linking program has been FAILED with code " << actionResult << std::endl;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		if ( infoLogLength > 0 ){
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}
		glDetachShader(programID, vertexShaderID);
		glDetachShader(programID, fragmentShaderID);
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
		return 0;
	}

	// glDetachShader(programID, vertexShaderID);
	// glDetachShader(programID, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}
