#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <common.hpp>
#include <nlablexer.h>
#include <nfunction.hpp>
#include <SimpleCriteria.hpp>
#include <imagedata.hpp>
#include "vbo.h"

#define NMATH_SUCCESS 0

// Vertex Shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 model;
    void main() {
        gl_Position = model * vec4(aPos, 1.0);
    }
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    } 
)";

struct FuncitonInputData {

	std::string functionText;
	float epsilon;
    // Specific for 2 variables function (3D space)
    // This is the domain in that we want to generate vertices, this is a subset of
    // domain of the input function
	float values[4];
};

// Shader Program
GLuint shaderProgram;

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Mouse movement variables
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float pitch = 0.0f;
float yaw = -90.0f;
bool firstMouse = true;

// Camera front vector
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

// Camera position
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

// Camera up vector
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    // for Y axis: We need to reverse since y-coordinate range from bottom
    // to top
    float yoffset = lastY - ypos;

    const float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Avoid gimbal lock
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate the new front vector using Quaternions
    glm::quat orientation = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));  
    cameraFront = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, -1.0f));

    lastX = xpos;
    lastY = ypos;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

std::vector<nmath::ImageData<float>*> generateIndices(const FuncitonInputData &input, int &errorCode) {
	nmath::NFunction<float> f;
	nmath::NLabLexer lexer;
	nmath::NLabParser<float> parser;
	int errorColumn;
    std::vector<nmath::ImageData<float>*> emptySpaces;

	errorCode = f.parse(input.functionText, &lexer, &parser);
	if(errorCode != NMATH_SUCCESS) {
		return emptySpaces;
	}

	std::vector<nmath::ImageData<float>*> spaces = f.getSpace(input.values, input.epsilon, true, false);
	errorCode = f.getErrorCode();
	if(errorCode != NMATH_SUCCESS) {
		for(auto i=0; i< spaces.size(); i++) {
			delete spaces[i];
		}
		return emptySpaces;
	}

    for (int i=0; i<spaces.size(); ++i) {
        nmath::ImageData<float>* imageData = spaces[i];
        unsigned int rowCount = imageData->getRowCount();
        imageData->generateIndices();
    }

    return spaces;
}

int main(int argc, const char* argv[]) {
	
    // init GLFW
    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFw" << std::endl;
        return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW windows
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "NautilusMath Demo", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Could not create windows" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window context current
    glfwMakeContextCurrent(window);

    // Set callback functions
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    //initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Shader Program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader program linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //Now, prepare VBO object
    int errorCode = 0;
    FuncitonInputData f;
    f.functionText = "f(x,y)=sin(x) + 3*y/4";
    f.values[0] = -1.0f;
    f.values[1] = 1.0f;
    f.values[2] = -1.0f;
    f.values[3] = -1.0f;
    f.epsilon = 0.01f;

    std::vector<nmath::ImageData<float>*> spaces = generateIndices(f, errorCode);

    if (errorCode != NMATH_SUCCESS) {
        glDeleteProgram(shaderProgram);
        return -1;
    }

    std::vector<VBO *> vboObjects(spaces.size());

    // Render loop
    while (!glfwWindowShouldClose(window)) {

    }

    // Cleanup
    for (int i=0; i<spaces.size(); i++) {
        if (vboObjects[i] != nullptr) {
            delete vboObjects[i];
        }
    }

    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();

    // Release function data
    for(auto i=0; i< spaces.size(); i++) {
        delete spaces[i];
    }

	return 0;
}