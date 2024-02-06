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
    layout (location = 1) in vec3 vertexNormal_modelspace;
    layout (location = 2) in vec4 vertexColor;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 mvp;
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform int useNormal;
    
    // Output data ; will be interpolated for each fragment.
    out vec3 Position_worldspace;
    out vec3 Normal_cameraspace;
    out vec4 Color_vertex;
    out vec3 EyeDirection_cameraspace;
    out vec3 LightDirection_cameraspace;
    out int oUseNormal;

    void main() {
        gl_Position = mvp * vec4(aPos, 1.0);
        // Position of the vertex, in worldspace : M * position
        Position_worldspace = (model * vec4(vertexPosition_modelspace, 1.0)).xyz;

        // Vector that goes from the vertex to the camera, in camera space.
        // In camera space, the camera is at the origin (0,0,0).
        vec3 vertexPosition_cameraspace = ( view * model * vec4(vertexPosition_modelspace, 1.0)).xyz;
        EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

        // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
        vec3 LightPosition_cameraspace = ( view * vec4(lightPos, 1.0)).xyz;
        LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

        oUserNormal = useNormal;
        if (useNormal == 1) {
            // Only correct if ModelMatrix does not scale the model ! 
            // Use its inverse transpose if not.
            Normal_cameraspace = (view * model * vec4(vertexNormal_modelspace, 0.0)).xyz;
        }

        Color_vertex = vertexColor;
    }
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 Position_worldspace;
    in vec3 Normal_cameraspace;
    in vec4 Color_vertex;
    in vec3 EyeDirection_cameraspace;
    in vec3 LightDirection1_cameraspace;
    in vec3 LightDirection2_cameraspace;
    in int oUseNormal;

    // Values that stay constant for the whole mesh.
    uniform mat4 MV;
    uniform vec3 lightPos_worldspace;
    uniform vec3 lightColor;

    struct LightSource {
        vec3 position_world;
        vec3 direction_camera;
        vec3 lightColor;
        vec3 diffuseColor;
        vec3 ambientColor;
        vec3 specularColor;
    };

    out vec4 FragColor;
    LightSource light;

    void main() {
        float LightPower = 20.0f;
        float specularPower = 10.0f;

        if(oUseNormal == 1) {
            vec3 MaterialDiffuseColor = Color_vertex;
            light.position_world = lightPos1_worldspace;
            light.direction_camera = LightDirection1_cameraspace;
            light.specularColor = vec3(0.9,0.9,0.9);
            light.ambientColor = vec3(0.4,0.4,0.4) * MaterialDiffuseColor;
            light.lightColor = lightColor;

            float distance = length( light.position_world - Position_worldspace );
            float distance2 = distance * distance;
            vec3 lightCamNormal = normalize( light.direction_camera );
            float cosTheta = clamp(dot(n, l), 0, 1);
            vec3 R = reflect(-lightCamNormal, n);
            float cosAlpha = clamp( dot( E,R ), 0,1 );
            vec3 diffuseReflection = MaterialDiffuseColor * light.lightColor * LightPower * cosTheta / distance2;
            vec3 specularReflection = light.specularColor * light.lightColor * 3 * specularPower * pow(cosAlpha,5) / distance2;

            FragColor = light.lightColor + light.ambientColor * 5.0;
        } else {
            FragColor = Color_vertex;
        }
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
const GLuint POSITION_LOCATION = 0;
const GLuint NORMAL_LOCATION = 1;
const GLuint COLOR_LOCATION = 2;

// Window dimensions
GLuint gWindowWidth = 800;
GLuint gWindowHeight = 600;

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
glm::mat4 gProjection;

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
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
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
    for (int i=0; i<spaces.size(); i++) {
        vboObjects[i] = new VBO(spaces[i], POSITION_LOCATION, COLOR_LOCATION, NORMAL_LOCATION);
    }

    GLuint mvpMatrixId = glGetUniformLocation(shaderProgram, "MVP");
	GLuint viewMatrixId = glGetUniformLocation(shaderProgram, "view");
	GLuint modelMatrixId = glGetUniformLocation(shaderProgram, "model");
	GLuint useNormalLocation = glGetUniformLocation(shaderProgram, "useNormal");
	GLuint lightPos1ID = glGetUniformLocation(shaderProgram, "lightPos_worldspace");
	GLuint lightColor1ID = glGetUniformLocation(shaderProgram, "lightColor");

	glm::vec3 lightPos = glm::vec3(7,7,7);
	glm::vec3 lightColor = glm::vec3(0.5f, 0.5f, 0.1f);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Process events
        glfwPollEvents();
        // Clear the color buffer and depth buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use the shader program
        glUseProgram(shaderProgram);

        glUniform3f(lightPos1ID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColor1ID, lightColor.x, lightColor.y, lightColor.z);

		// glUniform1i(shaderVarLocation.useNormalLocation, 10);
		// glUniform1i(shaderVarLocation.useLightingLocation, 10);


        for (int i=0; i<vboObjects.size(); i++) {
            vboObjects[i]->render();
        }

        // Swap the buffers
        glfwSwapBuffers(window);
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