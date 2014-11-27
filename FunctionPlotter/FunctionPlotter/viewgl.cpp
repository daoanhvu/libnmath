#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "errorconst.h"
#include "viewgl.h"
#include "resource.h"
#include "procedure.h"

using namespace Win;

/*
http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=2&list=1
*/

ViewGL::ViewGL(HINSTANCE hInstance, HWND parent, const wchar_t *name, int width, int height,
	DWORD clsStyle, DWORD wStyle, unsigned char majorv, unsigned char minorv): mInstance(hInstance), 
	mParentHandle(parent), mHdc(0), mHglrc(0), windowWidth(width), windowHeight(height), 
	winStyle(wStyle), winStyleEx(0), mMajorVersion(majorv), mMinorVersion(minorv){
	//copy class name and window name
	wcsncpy(this->title, name, MAX_STRING - 1);
	wcsncpy(this->className, name, MAX_STRING - 1);

	//populate window class struct
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = clsStyle;                                     // class styles: CS_OWNDC, CS_PARENTDC, CS_CLASSDC, CS_GLOBALCLASS, ...
	winClass.lpfnWndProc = Win::windowProcedure;                  // pointer to window procedure
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = mInstance;                              // owner of this class
	winClass.hIcon = LoadIcon(mInstance, IDI_APPLICATION);   // default icon
	winClass.hIconSm = 0;
	winClass.hCursor = LoadCursor(0, IDC_ARROW);              // default arrow cursor
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);   // default white brush
	winClass.lpszMenuName = 0;
	winClass.lpszClassName = className;
	winClass.hIconSm = LoadIcon(mInstance, IDI_APPLICATION);   // default small icon

	::RegisterClassEx(&winClass);
}


ViewGL::~ViewGL(void) {
}

int ViewGL::init() {
	HGLRC tempRContext;
	WNDCLASSEX winCls;
	//populate window class struct

	winCls.cbSize = sizeof(WNDCLASSEX);
	winCls.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	winCls.lpfnWndProc = (WNDPROC)Win::fakeWindowProcedure;                  // pointer to window procedure
	winCls.cbClsExtra = 0;
	winCls.cbWndExtra = 0;
	winCls.hInstance = mInstance;                              // owner of this class
	winCls.hIcon = LoadIcon(mInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	winCls.hIconSm = LoadIcon(mInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	winCls.hCursor = LoadCursor(0, IDC_ARROW);              // default arrow cursor
	winCls.hbrBackground = (HBRUSH)(COLOR_MENUBAR + 1);
	winCls.lpszMenuName = 0;
	winCls.lpszClassName = L"FakeGLWindow";
	winCls.hIconSm = LoadIcon(mInstance, IDI_APPLICATION);   // default small icon
	if (!::RegisterClassEx(&winCls)) return 1;
	//Make a fake window to get glew working
	HWND hFakeW = ::CreateWindow(L"FakeGLWindow", L"FAKE",
		WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_CLIPCHILDREN,
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
		NULL, mInstance, NULL);
	mHdc = GetDC(hFakeW);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ::ChoosePixelFormat(mHdc, &pfd);

	if (nPixelFormat == 0) {
		MessageBox(0, L"Cannot set a suitable pixel format.", L"Error", MB_ICONEXCLAMATION | MB_OK);
		ReleaseDC(hFakeW, mHdc);
		return 2;
	}
	::SetPixelFormat(mHdc, nPixelFormat, &pfd);

	tempRContext = ::wglCreateContext(mHdc);
	wglMakeCurrent(mHdc, tempRContext);

	//Initialize GLEW
	glewExperimental = TRUE;
	if (glewInit() != GLEW_OK)
		return 3;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tempRContext);
	//ReleaseDC(hFakeW, mHdc);
	DestroyWindow(hFakeW);

	return NFP_OK;
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

HWND ViewGL::createWindow(void *controller) {
	//Create actual window for OpenGL here
	mHandle = CreateWindowEx(winStyleEx, className, title, winStyle, x, y,
		windowWidth, windowHeight, mParentHandle, NULL, mInstance, (LPVOID)controller);

	return mHandle;
}

// create OpenGL rendering context
int ViewGL::createGLContext(int colorBits, int depthBits, int stencilBits) {
	int nPixelFormat;
	int nNumFormat;
	
	if (init() != NFP_OK) return NFP_ERROR_INIT_GLEW_FAIL;

	//retrieve a handle to the device context
	mHdc = GetDC(mHandle);

	//Set pixel format
	PIXELFORMATDESCRIPTOR pfd;

	if (mMajorVersion <= 2) {
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		nPixelFormat = ::ChoosePixelFormat(mHdc, &pfd);

		if (nPixelFormat == 0) {
			::MessageBox(0, L"Cannot set a suitable pixel format.", L"Error", MB_ICONEXCLAMATION | MB_OK);
			::ReleaseDC(mHandle, mHdc);
			return NFP_ERROR_PIXEL_FORMAT_FAIL;
		}

		::SetPixelFormat(mHdc, nPixelFormat, &pfd);
		mHglrc = wglCreateContext(mHdc);

		if (mHglrc) wglMakeCurrent(mHdc, mHglrc);
	}
	else {
		if (WGLEW_ARB_create_context && WGLEW_ARB_pixel_format) {
			const int iPixelFormatAttribList[] = {	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
													WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
													WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
													WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
													WGL_COLOR_BITS_ARB, 32,
													WGL_DEPTH_BITS_ARB, 24,
													WGL_STENCIL_BITS_ARB, 8,
													0 // End of attributes list
												};

			int contextAttribs[] = {	WGL_CONTEXT_MAJOR_VERSION_ARB, mMajorVersion,
										WGL_CONTEXT_MINOR_VERSION_ARB, mMinorVersion,
										WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
										0 };

			wglChoosePixelFormatARB(mHdc, iPixelFormatAttribList, NULL, 1, &nPixelFormat, (UINT*)&nNumFormat);
			if (!SetPixelFormat(mHdc, nPixelFormat, &pfd)) return NFP_ERROR_PIXEL_FORMAT_FAIL;
			mHglrc = wglCreateContextAttribsARB(mHdc, 0, contextAttribs);
			if (mHglrc) {
				wglMakeCurrent(mHdc, mHglrc);
				mProgramID = loadShader("D:\\Documents\\testapp\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.vertextsharder",
					"D:\\Documents\\testapp\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.fragmentshader");
				//mProgramID = loadShader("D:\\projects\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.vertextsharder",
				//	"D:\\projects\\libnmath\\FunctionPlotter\\FunctionPlotter\\shaders\\fplotter.fragmentsharder");
				if (mProgramID <= 0) {
					return NFP_ERROR_GLSL_PROGRAM_FAIL;
				}

				mPositionID = glGetAttribLocation(mProgramID, "position");
				mEnableLightID = glGetUniformLocation(mProgramID, "enableLight");
			}
			
		}
	}

	//Or better yet, use the GL3 way to get the version number
	/*
	int glVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
	const unsigned char *versionString = glGetString(GL_VERSION);
	*/

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	return NFP_OK;
}

void ViewGL::closeContext() {
	if(!mHdc && !mHglrc) return;

	::wglMakeCurrent(0, 0);
	::wglDeleteContext(mHglrc);
	::ReleaseDC(mHandle, mHdc);
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

void ViewGL::show(int shw) {
	::ShowWindow(mHandle, shw);
	::UpdateWindow(mHandle);
}

GLuint ViewGL::loadShader(const char *vertexShaderFile, const char *fragmentShaderFile) {
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