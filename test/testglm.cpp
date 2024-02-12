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
#include <shader.h>
#include <vbo_mapper.h>

#define NMATH_SUCCESS 0

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
GLuint gWindowWidth = 1000;
GLuint gWindowHeight = 800;

// Mouse movement variables
float lastX = gWindowWidth / 2.0f;
float lastY = gWindowHeight / 2.0f;
float pitch = 0.0f;
float yaw = -90.0f;
bool firstMouse = true;

// Camera front vector
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
// Camera position
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
// Camera up vector
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 centerPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::mat4 gProjection;
const auto xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
const auto yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
const auto zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
        lastX = xpos;
        lastY = ypos;
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
    gWindowWidth = width;
    gWindowHeight = height;
    gProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
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
    std::cout << "Done getting space from function.\n" << std::endl;

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
    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "NautilusMath Demo", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Could not create windows" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set up the view matrix and projection matrix (you might want to set these up according to your needs)
    glm::mat4 view = glm::lookAt(cameraPos, centerPos, cameraUp);
    gProjection = glm::perspective(glm::radians(45.0f), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 100.0f);

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

    glm::mat4 ModelMatrix, MVP;
	ShaderVarLocation shaderVarLocation;
	shaderProgram = loadShaders( "shaders/vertex_test.shader", "shaders/fragment_test.shader");

    if (shaderProgram == GL_ZERO) {
        glfwTerminate();
        std::cerr << "Load shaders FAILED!" << std::endl;
        return -1;
    }

    shaderVarLocation.positionLocation  = 0;
    shaderVarLocation.normalLocation    = 1;
    shaderVarLocation.colorLocation     = 2;

    //Now, prepare VBO object
    int errorCode = 0;
    FuncitonInputData f;
    f.functionText = "f(x,y)=sin(x) + 3*y/4";
    f.values[0] = -1.0f;
    f.values[1] = 1.0f;
    f.values[2] = -1.0f;
    f.values[3] = -1.0f;
    f.epsilon = 0.01f;

    // Prepare Coordinator VBO
    float coordinatorData[] = {
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
    VBO *coordinator = fromDataToVBO(coordinatorData, 6, 6, shaderVarLocation, false);

    // std::vector<nmath::ImageData<float>*> spaces = generateIndices(f, errorCode);
    // if (errorCode != NMATH_SUCCESS) {
    //     std::cerr << "Failed calculate spaces from function." << std::endl;
    //     glDeleteProgram(shaderProgram);
    //     return -1;
    // }

    // std::cout << "Number of space generated: " << spaces.size() << std::endl;

    // std::vector<VBO *> vboObjects(spaces.size());
    // for (int i=0; i<spaces.size(); i++) {
    //     vboObjects[i] = fromImageDataToVBO(spaces[i], shaderVarLocation);
    // }

    std::cout << "Getting shader's locations...\n" << std::endl;
    shaderVarLocation.perspectiveMatrixId = glGetUniformLocation(shaderProgram, "P");
	shaderVarLocation.viewMatrixId = glGetUniformLocation(shaderProgram, "V");
	shaderVarLocation.modelMatrixId = glGetUniformLocation(shaderProgram, "M");
	shaderVarLocation.useNormalID = glGetUniformLocation(shaderProgram, "useNormal");
	shaderVarLocation.lightPos1ID = glGetUniformLocation(shaderProgram, "lightPos");
	shaderVarLocation.lightColor1ID = glGetUniformLocation(shaderProgram, "lightColor");

	glm::vec3 lightPos = glm::vec3(2.7f,2.0f,3.0f);
	glm::vec3 lightColor = glm::vec3(0.5f, 0.5f, 0.1f);

    std::cout << "Start render loop...\n" << std::endl;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Process events
        glfwPollEvents();
        // Clear the color buffer and depth buffer
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use the shader program
        glUseProgram(shaderProgram);

        glUniform3f(shaderVarLocation.lightPos1ID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(shaderVarLocation.lightColor1ID, lightColor.x, lightColor.y, lightColor.z);
        glUniformMatrix4fv(shaderVarLocation.perspectiveMatrixId, 1, GL_FALSE, glm::value_ptr(gProjection));
        glUniformMatrix4fv(shaderVarLocation.viewMatrixId, 1, GL_FALSE, glm::value_ptr(view));

        // Create a rotation quaternion based on mouse movement
        // glm::quat rotation = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));

        coordinator->applyRotation(glm::radians(pitch), xAxis, glm::radians(yaw), yAxis, 0.0f, zAxis);
        coordinator->render(shaderVarLocation);
        // for (int i=0; i<vboObjects.size(); i++) {
        //     vboObjects[i]->applyRotation(glm::radians(yaw), xAxis, glm::radians(pitch), yAxis, 0.0f, zAxis);
        //     vboObjects[i]->render(shaderVarLocation, gProjection, view);
        // }

        // Swap the buffers
        glfwSwapBuffers(window);
    }
    // Clean up
    delete coordinator;
    // for (int i=0; i<spaces.size(); i++) {
    //     if (vboObjects[i] != nullptr) {
    //         delete vboObjects[i];
    //     }
    // }

    glDisableVertexAttribArray(shaderVarLocation.positionLocation);
    glDisableVertexAttribArray(shaderVarLocation.normalLocation);
    glDisableVertexAttribArray(shaderVarLocation.colorLocation);
    glDeleteProgram(shaderProgram);
    std::cout << "Going to terminate window...\n" << std::endl;
    // Terminate GLFW
    glfwTerminate();
    std::cout << "Terminated window...\n" << std::endl;
    // Release function data
    // for(auto i=0; i< spaces.size(); i++) {
    //     delete spaces[i];
    // }

	return 0;
}