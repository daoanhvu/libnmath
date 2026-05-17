#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shaders_utils.h"
#include "vbo_objects.h"
#include "common.hpp"
#include "imagedata.hpp"
#include "nlablexer.h"
#include "nfunction.hpp"
#include "SimpleCriteria.hpp" 
#include "command_handler.h"
#include "windows_helper.h"

struct MeshBufferIndices {
  GLuint VBO;
  GLuint colorVBO;
  GLuint EBO;
  GLuint VAO;
  int normalOffset;
  int indexLength;
};

// Shader Program
GLuint shaderProgram;

// Window dimensions
const GLuint WIDTH = 1400, HEIGHT = 800;

// Mouse movement variables
double lastX = WIDTH / 2.0;
double lastY = HEIGHT / 2.0;
bool firstMouse = true;

// Fixed camera setup
// const glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
const glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Rotation variables
float mouseSpeed = 0.3f; // Reduced for smoother rotation
bool isDragging = false;

GLRotationParameters rotationParams;

// Add these global variables for the textbox
char inputText[256] = "";
bool textboxActive = false;

// Add lighting variables
bool useLighting = true;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);  // Light position
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // White light

bool mousePressed = false;

// Function to handle mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (ImGui::GetIO().WantCaptureMouse) {
      return;  // Skip processing if ImGui is using the mouse
    }

    if (!mousePressed) {
      return;
    }

    if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
      return;
    }

    float deltaX = float(xpos - lastX);
    float deltaY = float(lastY - ypos);
    lastX = xpos;
    lastY = ypos;

    rotationParams.rotationY += deltaX * mouseSpeed;
    rotationParams.rotationX += deltaY * mouseSpeed;

    // Constrain the pitch to avoid flipping
    if (rotationParams.rotationX > 89.0f)
      rotationParams.rotationX = 89.0f;
    if (rotationParams.rotationX < -89.0f)
      rotationParams.rotationX = -89.0f;
}

// Function to handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Function to handle mouse button events
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;  // Skip processing if ImGui is using the mouse
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      mousePressed = true;
      firstMouse = true;
    } else if (action == GLFW_RELEASE) {
      mousePressed = false;
    }
  }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
      std::cerr << "Failed to initialize GLFW" << std::endl;
      return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Nautilus Function Plotter 1.1", nullptr, nullptr);
    if (window == nullptr) {
      std::cerr << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set the callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Read and compile shaders
    std::string vertexCode = readShaderFile("shaders/vertex.glsl");
    std::string fragmentCode = readShaderFile("shaders/fragment.glsl");
    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // Vertex Shader
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    // Fragment Shader
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    // Shader Program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    // Disable face culling since we want to see both sides of the surface
    glDisable(GL_CULL_FACE);

    // Set up viewport
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Print OpenGL version and renderer info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

    ShaderVarLocation locations;

    glUseProgram(shaderProgram);
    locations.positionLocation = glGetAttribLocation(shaderProgram, "aPos");
    locations.normalLocation = glGetAttribLocation(shaderProgram, "aNormal");
    locations.colorLocation = glGetAttribLocation(shaderProgram, "aColor");
    
    locations.perspectiveMatrixId = glGetUniformLocation(shaderProgram, "projection");
    locations.viewMatrixId = glGetUniformLocation(shaderProgram, "view");
    locations.modelMatrixId = glGetUniformLocation(shaderProgram, "model");
    locations.normalMatrixId = glGetUniformLocation(shaderProgram, "uNormalMatrix");
    locations.viewPosId = glGetUniformLocation(shaderProgram, "uViewPos");
    locations.useLightingLocation = glGetUniformLocation(shaderProgram, "uUseLighting");
    locations.lightPos1ID = glGetUniformLocation(shaderProgram, "uLightPos");
    locations.lightColor1ID = glGetUniformLocation(shaderProgram, "uLightColor");

    glm::vec3 cameraPos(0.0f, 0.0f, 16.0f); 
    glm::mat4 view = glm::lookAt(
      cameraPos,                          // Camera position
      glm::vec3(0.0f, 0.0f, 0.0f),       // Look at point
      glm::vec3(0.0f, 1.0f, 0.0f)        // Up vector
    );

    // Coordinate system
    GLCoordinates axisVBO = initCoordinates(locations);
    CommandHandler commandHandler(locations);
    bool shouldRotate = false;

    // Generate mesh and set up vertex data and buffers
    std::vector<VboObject*> meshes;
    float values[4] = {-1.8f, 1.8f, -1.8f, 1.8f};
    float epsilon = 0.1f;
    bool shouldReInitMeshes = false;
    glm::vec4 objectColor(0.3f, 0.5f, 0.2f, 1.0f);
    float arrColor[4];

    // Render loop
    while (!glfwWindowShouldClose(window)) {
      handleRotation(window, rotationParams);
      
      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // Create ImGui window with controls
      ImGui::Begin("Controls");
      
      // Add textbox
      if (ImGui::InputText("Enter text", inputText, IM_ARRAYSIZE(inputText), ImGuiInputTextFlags_EnterReturnsTrue)) {
        // TODO: Handle text input (when Enter is pressed)
        std::cout << "Entered text: " << inputText << std::endl;
        shouldReInitMeshes = true;
        arrColor[0] = objectColor.r;
        arrColor[1] = objectColor.g;
        arrColor[2] = objectColor.b;
        arrColor[3] = objectColor.a;
        commandHandler.handleCommand(inputText, arrColor, meshes);
        if (commandHandler.getErrorCode() == NMATH_NO_ERROR) {
          shouldReInitMeshes = false;
        } else {
          std::cerr << "Error in command: " << commandHandler.getErrorCode() << " at column: " << commandHandler.getErrorColumn() << std::endl;
        }
      }
      ImGui::ColorEdit4("Object Color", glm::value_ptr(objectColor));

      // Add lighting controls
      ImGui::Checkbox("Use Lighting", &useLighting);
      ImGui::Checkbox("Rotate", &shouldRotate);

      if (useLighting) {
        ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
        ImGui::DragFloat3("Light Position", glm::value_ptr(lightPos), 0.1f);
        ImGui::DragFloat3("Camera Position", glm::value_ptr(cameraPos), 0.2f);
        view = glm::lookAt(
          cameraPos,                          // Camera position
          glm::vec3(0.0f, 0.0f, 0.0f),       // Look at point
          glm::vec3(0.0f, 1.0f, 0.0f)        // Up vector
        );
      }

      ImGui::End();

      // Clear the color buffer and depth buffer
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // Use the shader program
      glUseProgram(shaderProgram);

      // Set transformation matrices
      glm::mat4 globalModel = glm::mat4(1.0f);
      globalModel = glm::rotate(globalModel, glm::radians(rotationParams.rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
      globalModel = glm::rotate(globalModel, glm::radians(rotationParams.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
      glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

      // Set transformation matrices in shader
      glUniformMatrix4fv(locations.viewMatrixId, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(locations.perspectiveMatrixId, 1, GL_FALSE, glm::value_ptr(projection));
      // Calculate and set view position for specular lighting // Camera position
      glUniform3fv(locations.viewPosId, 1, glm::value_ptr(cameraPos));

      renderCoordinates(axisVBO, locations, globalModel);
      renderMeshes(meshes, locations, globalModel, useLighting ? 1 : 0, lightPos, lightColor);

      if (shouldReInitMeshes) {
        for(auto i=0; i< meshes.size(); i++) {
          meshes[i]->release();
          delete meshes[i];
        }
        meshes.clear();
        // generateMeshAndIndices(inputText, values, epsilon, locations, meshes);
        // generateRoundedCone(1.0f, 0.5f, 0.2f, 10, 8, 0.3f, 0.5f, 0.2f, 1.0f, locations, meshes);
        for(auto i=0; i< meshes.size(); i++) {
          meshes[i]->setupArrayAttributes();
        }
        shouldReInitMeshes = false;
      }

      // Check for OpenGL errors
      // GLenum err;
      // while ((err = glGetError()) != GL_NO_ERROR) {
      //   std::cerr << "OpenGL error: " << err << std::endl;
      // }

      // Render ImGui
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      // Swap the buffers
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup meshes
    for(auto i=0; i< meshes.size(); i++) {
      meshes[i]->release();
      delete meshes[i];
    }
    releaseCoordinates(axisVBO);

    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
