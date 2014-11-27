#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "viewgl.h"
#include "resource.h"

using namespace Win;

/*
http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=2&list=1
*/

ViewGL::ViewGL(void): mHdc(0), mHglrc(0) {
}


ViewGL::~ViewGL(void) {
}

int ViewGL::init() {
	return 0;
}

bool ViewGL::setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits) {
	PIXELFORMATDESCRIPTOR pfd;
	int pixelFormat = findPixelFormat(hdc, colorBits, depthBits, tencilBits);
	if(pixelFormat == 0)
		return false;

	::DescribePixelFormat(hdc, pixelFormat, sizeof(pfd), &pfd);

	if(!::SetPixelFormat(hdc, pixelFormat, &pfd)) {
		return false;
	}

	return true;
}

int ViewGL::findPixelFormat(HDC hdc, int colorBits, int depthBits, int stencilBits) {
	int currMode = 0;
	int bestMode = 0;
	int currScore = 0;
	int bestScore = 0;
	PIXELFORMATDESCRIPTOR pfd;

	for(currMode=1; ::DescribePixelFormat(hdc, currMode, sizeof(pfd), &pfd) > 0; currMode++) {
		//ignore if cannot support opengl or cannot render into a window
		if(!(pfd.dwFlags & PFD_SUPPORT_OPENGL) || !(pfd.dwFlags & PFD_DRAW_TO_WINDOW))
			continue;

		// ignore if cannot support rgba mode
        if((pfd.iPixelType != PFD_TYPE_RGBA) || (pfd.dwFlags & PFD_NEED_PALETTE))
            continue;

        // ignore if not double buffer
        if(!(pfd.dwFlags & PFD_DOUBLEBUFFER))
            continue;

		// colour bits
        if(pfd.cColorBits >= colorBits) ++currScore;

        // depth bits
        if(pfd.cDepthBits >= depthBits) ++currScore;

        // stencil bits
        if(pfd.cStencilBits >= stencilBits) ++currScore;

        // alpha bits
        if(pfd.cAlphaBits > 0) ++currScore;

        // check if it is best mode so far
        if(currScore > bestScore) {
            bestScore = currScore;
            bestMode = currMode;
        }
	}

	return bestMode;
}

// create OpenGL rendering context
bool ViewGL::createContext(HWND handle, int colorBits, int depthBits, int stencilBits) {
	HGLRC tempRContext;

	//retrieve a handle to the device context
	mHdc = ::GetDC(handle);

	//Set pixel format
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	//pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iLayerType = PFD_TYPE_RGBA;

	int nPixelFormat = ::ChoosePixelFormat(mHdc, &pfd);

	if(nPixelFormat == 0) {
		::MessageBox(0, L"Cannot set a suitable pixel format.", L"Error", MB_ICONEXCLAMATION | MB_OK);
		::ReleaseDC(handle, mHdc);
		return false;
	}
	::SetPixelFormat(mHdc, nPixelFormat, &pfd);

	tempRContext = ::wglCreateContext(mHdc);
	::wglMakeCurrent(mHdc, tempRContext);

	//Initialize GLEW
	glewExperimental = TRUE;
	if (glewInit() != GLEW_OK)
		return false;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tempRContext);

	int attribs[] = {	WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
						WGL_CONTEXT_MINOR_VERSION_ARB, 1,
						WGL_CONTEXT_FLAGS_ARB, 0,
						0 };
	
	mHglrc = wglCreateContextAttribsARB(mHdc, 0, attribs);
	::wglMakeCurrent(NULL, NULL);
	::wglDeleteContext(mHglrc);
	::wglMakeCurrent(mHdc, mHglrc);

	//Checking GL version
	const GLubyte *glVersionString = glGetString(GL_VERSION);
	//Or better yet, use the GL3 way to get the version number
	int glVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

	const unsigned char *versionString = glGetString(GL_VERSION);

	if (mHglrc == NULL) return false;

	//mProgramID = loadShader("D:\\Documents\\testapp\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.vertextsharder",
	//	"D:\\Documents\\testapp\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.fragmentshader");
	mProgramID = loadShader("D:\\projects\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.vertextsharder",
		"D:\\projects\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.fragmentsharder");
	if (mProgramID <= 0) {
		return false;
	}

	mPositionID = glGetAttribLocation(mProgramID, "position");
	mEnableLightID = glGetUniformLocation(mProgramID, "enableLight");

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//::ReleaseDC(handle, mHdc);
	return true;
}

void ViewGL::closeContext(HWND handle) {
	if(!mHdc && !mHglrc) return;

	::wglMakeCurrent(0, 0);
	::wglDeleteContext(mHglrc);
	::ReleaseDC(handle, mHdc);
	mHdc = 0;
	mHglrc = 0;
}

void ViewGL::swapBuffers() {
	::SwapBuffers(mHdc);
}

void ViewGL::setViewport(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);

	float aspectRatio = (float)width / height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, aspectRatio, 0.1f, 20.0f);
	glMatrixMode(GL_MODELVIEW);
}

bool ViewGL::resetViewportIfNeeded() {
	if (!windowResized) return false;

	setViewport(windowWidth, windowHeight);
	return true;
}

void ViewGL::resizeWindow(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	windowResized = true;
}

GLuint ViewGL::loadShader(const char *vertexShaderFile, const char *fragmentShaderFile) {
	//GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	//GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	//PFNGLCREATESHADERPROC glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShaderARB");
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint result;
	int infoLen;

	//Read the Vertex shader
	std::string vertexShaderCode;
	std::ifstream vertexShaderStream(vertexShaderFile, std::ios::in);

	if (!vertexShaderStream.is_open()) {
		return 0;
	}

	std::string line = "";
	while (getline(vertexShaderStream, line)) {
		vertexShaderCode += line + "\n";
	}
	vertexShaderStream.close();

	// Read the Fragment Shader code from the file
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderStream(fragmentShaderFile, std::ios::in);
	if (fragmentShaderStream.is_open()){
		line = "";
		while (getline(fragmentShaderStream, line))
			fragmentShaderCode += line + "\n";
		fragmentShaderStream.close();
	}

	//Compile vertex shader
	const char *vertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
	glCompileShader(vertexShaderID);
	//Check vertex shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLen);
	if (infoLen > 0){
		std::vector<char> vertexShaderErrorMessage(infoLen + 1);
		glGetShaderInfoLog(vertexShaderID, infoLen, NULL, &vertexShaderErrorMessage[0]);
		printf("%s\n", &vertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	//printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = fragmentShaderCode.c_str();
	glShaderSource(fragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLen);
	if (infoLen > 0){
		std::vector<char> fragmentShaderErrorMessage(infoLen + 1);
		glGetShaderInfoLog(fragmentShaderID, infoLen, NULL, &fragmentShaderErrorMessage[0]);
		printf("%s\n", &fragmentShaderErrorMessage[0]);
	}

	//Link program
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen);
	if (infoLen > 0){
		std::vector<char> programErrorMessage(infoLen + 1);
		glGetProgramInfoLog(programID, infoLen, NULL, &programErrorMessage[0]);
		printf("%s\n", &programErrorMessage[0]);
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}