#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

void mouseCallback(GLFWwindow *windows, double xpos, double ypos) {
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

	return 0;
}