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
#include "function_utils.h"

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
float mouseSpeed = 0.003f; // Reduced for smoother rotation
bool isDragging = false;
glm::mat4 rotationMatrix = glm::mat4(1.0f); // Initialize as identity matrix

// Add these global variables for the textbox
char inputText[256] = "";
bool textboxActive = false;

// Add lighting variables
bool useLighting = true;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);  // Light position
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // White light

// Variables for rotation
float rotationX = 0.0f;
float rotationY = 0.0f;
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

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.5f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    rotationY += xoffset;
    rotationX += yoffset;

    // Constrain the pitch to avoid flipping
    if (rotationX > 89.0f)
      rotationX = 89.0f;
    if (rotationX < -89.0f)
      rotationX = -89.0f;
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

// In your main rendering loop
void handleRotation(GLFWwindow* window) {
    if (!isDragging) return;
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    // Calculate rotation angles based on mouse movement
    float deltaX = float(xpos - lastX);
    float deltaY = float(ypos - lastY);
    
    // Convert mouse movement to rotation angles
    float horizontalAngle = -mouseSpeed * deltaY;  // Rotation around X-axis (pitch)
    float verticalAngle = -mouseSpeed * deltaX;    // Rotation around Y-axis (yaw)
    
    // Use fixed world-space axes for rotation
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(1.0f, 0.0f, 0.0f));  // Fixed X-axis
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(0.0f, 1.0f, 0.0f));    // Fixed Y-axis
    
    // Apply rotations in order: first around Y-axis, then around X-axis
    rotationMatrix = rotX * rotY * rotationMatrix;
    
    lastX = xpos;
    lastY = ypos;
}

struct GLCoordinates {
  GLuint axisVao;
  GLuint axisVbo;
  GLuint axisColorVbo;
};

GLCoordinates initCoordinates(ShaderVarLocation locations) {
    // Initialize the coordinate system here
    // This function can be used to set up any initial state for the coordinate system
    float axisVertices[] = {
        // X-axis
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        // Y-axis
        0.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // Z-axis
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 1.0f
    };
    float axisColors[] = {
        // X-axis color (red)
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        // Y-axis color (green)
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        // Z-axis color (blue)
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    };
    // VboObject* axisVBO = new VboObject(locations, GL_LINES);
    // axisVBO->initialize(axisVertices, 18, 6, axisColors, nullptr, 0, -1);
    GLCoordinates result;
    GLuint axisVao;
    GLuint axisVbo;
    GLuint axisColorVbo;
    glGenVertexArrays(1, &axisVao);
    glBindVertexArray(axisVao);

    glGenBuffers(1, &axisVbo);
    glBindBuffer(GL_ARRAY_BUFFER, axisVbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), axisVertices, GL_STATIC_DRAW);
    // Set up position attribute (location 0)
    glBindBuffer(GL_ARRAY_BUFFER, axisVbo);
    int strideInBytes = 3 * sizeof(float);
    glVertexAttribPointer(locations.positionLocation, 3, GL_FLOAT, GL_FALSE, strideInBytes, (void*)0);
    glEnableVertexAttribArray(locations.positionLocation);

    strideInBytes = 4 * sizeof(float);
    glGenBuffers(1, &axisColorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, axisColorVbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), axisColors, GL_STATIC_DRAW);

    result.axisVao = axisVao;
    result.axisVbo = axisVbo;
    result.axisColorVbo = axisColorVbo;
    return result;
}

void renderCoordinates(const GLCoordinates &axes, ShaderVarLocation locations, glm::mat4 model) {
  glUniform1i(locations.useLightingLocation, 0);
  // Render the coordinate system here
  glBindVertexArray(axes.axisVao);
  
  glBindBuffer(GL_ARRAY_BUFFER, axes.axisVbo);
  glEnableVertexAttribArray(locations.positionLocation);
  glVertexAttribPointer(locations.positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        3 * sizeof(float),
                        reinterpret_cast<void*>(0));
  
  glBindBuffer(GL_ARRAY_BUFFER, axes.axisColorVbo);
  glEnableVertexAttribArray(locations.colorLocation);
  glVertexAttribPointer(locations.colorLocation, //Attribute index
                        4,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        4 * sizeof(float),
                        reinterpret_cast<void*>(0));
    

    glDrawArrays(GL_LINES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisableVertexAttribArray(locations.positionLocation);
    glDisableVertexAttribArray(locations.normalLocation);
    glDisableVertexAttribArray(locations.colorLocation);
}

void releaseCoordinates(GLCoordinates &axes) {
  glDeleteVertexArrays(1, &axes.axisVao);
  glDeleteBuffers(1, &axes.axisVbo);
  glDeleteBuffers(1, &axes.axisColorVbo);
}

void renderMeshes(std::vector<VboObject*>& meshes, ShaderVarLocation locations, glm::mat4 model) {
  // Set lighting uniforms
  glUniform1i(locations.useLightingLocation, useLighting ? 1 : 0);
  glUniform3fv(locations.lightPos1ID, 1, glm::value_ptr(lightPos));
  glUniform3fv(locations.lightColor1ID, 1, glm::value_ptr(lightColor));

  // Calculate and set normal matrix
  glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
  glUniformMatrix3fv(locations.normalMatrixId, 1, GL_FALSE, glm::value_ptr(normalMatrix));
  for (auto& mesh : meshes) {
      mesh->render();
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

    // Generate mesh and set up vertex data and buffers
    std::vector<VboObject*> meshes;
    float values[4] = {-1.8f, 1.8f, -1.8f, 1.8f};
    float epsilon = 0.1f;
    bool shouldReInitMeshes = false;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
      handleRotation(window);
      
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
      }

      // Add lighting controls
      ImGui::Checkbox("Use Lighting", &useLighting);
      
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
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
      glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

      // Set transformation matrices in shader
      glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(locations.viewMatrixId, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(locations.perspectiveMatrixId, 1, GL_FALSE, glm::value_ptr(projection));
      // Calculate and set view position for specular lighting // Camera position
      glUniform3fv(locations.viewPosId, 1, glm::value_ptr(cameraPos));

      renderCoordinates(axisVBO, locations, model);
      renderMeshes(meshes, locations, model);

      if (shouldReInitMeshes) {
        for(auto i=0; i< meshes.size(); i++) {
          meshes[i]->release();
          delete meshes[i];
        }
        meshes.clear();
        // generateMeshAndIndices(inputText, values, epsilon, locations, meshes);
        generateRoundedCone(1.0f, 0.5f, 0.2f, 10, 8, 0.3f, 0.5f, 0.2f, 1.0f, locations, meshes);
        for(auto i=0; i< meshes.size(); i++) {
          meshes[i]->setupArrayAttributes();
        }
        shouldReInitMeshes = false;
      }

      // Check for OpenGL errors
      GLenum err;
      while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
      }

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
