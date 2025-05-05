#include "command_handler.h"
#include <stack>
#include <iostream>

#ifdef _BUILT_WITH_OPENGL
#include <GL/glew.h>
#else
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009
#endif

nmath::Token* transformToken(nmath::Token* token) {
  // Transform the token as needed
  if (token == nullptr) {
    return nullptr;
  }

  if (token->type == NAME) {
    if (strcmp(token->text, "cone") == 0) {
      token->type = TYPE_CONE;
    } else if (strcmp(token->text, "up") == 0) {
      token->type = TYPE_ACTION_UP;
    } else if (strcmp(token->text, "down") == 0) {
      token->type = TYPE_ACTION_DOWN;
    } else if (strcmp(token->text, "left") == 0) {
      token->type = TYPE_ACTION_LEFT;
    } else if (strcmp(token->text, "right") == 0) {
      token->type = TYPE_ACTION_RIGHT;
    } else if (strcmp(token->text, "rotate") == 0) {
      token->type = TYPE_ACTION_ROTATE;
    } else if (strcmp(token->text, "move") == 0) {
      token->type = TYPE_ACTION_MOVE;
    }
  }

  return token;
}

void CommandHandler::handleCommand(const std::string& commandText, 
        const float *color, std::vector<VboObject *> &results) {
  // Implementation of command handling logic
  vector<nmath::Token*> tokens;
  int lastMeanIdx = -1;
  size_t tokenCount = lexer.lexicalAnalysis(commandText.c_str(), commandText.length(), false, 0, 
        tokens, &lastMeanIdx);

  // TODO: Parsing the tokens to create a function or object
  if (tokenCount == 0) {
    std::cerr << "Error: No tokens found in the command." << std::endl;
    return;
  }

  std::vector<nmath::NMAST<float>* > variables;
  parser.functionNotation(tokens, 0, variables, &errorCode, &errorColumn);
  if (errorCode == NMATH_NO_ERROR) {
    float values[4] = {-1.8f, 1.8f, -1.8f, 1.8f};
    std::cout << "Creating meshes" << std::endl;
    createMesh(tokens, values, 0.2f, results);
    return;
  } else {
    std::cout << "Processing command" << std::endl;
    parseCommand(tokens, color, results);
    return;
  }

  // Clean up the tokens
  std::cout << "Cleaning up ..." << std::endl;
  for(int i=0; i<tokens.size(); i++) {
    delete tokens[i];
  }
}

// Private member variables and functions
void CommandHandler::parseCommand(const vector<nmath::Token*> &tokens, const float *color, std::vector<VboObject *> &results) {
  // Implementation of command parsing logic
  int currentIndex = 0;
  size_t tokenCount = tokens.size();
  std::stack<nmath::Token*> tokenStack;
  std::vector<nmath::Token*> tempList;
  while (currentIndex < tokenCount) {
    nmath::Token* token = transformToken(tokens[currentIndex]);
    switch(token->type) {
      case TYPE_CONE:
        // Handle cone function
        tokenStack.push(token);
        break;
      case TYPE_ACTION_UP:
        // Handle action up
        break;
      case TYPE_ACTION_DOWN:
        // Handle action down
        break;
      case TYPE_ACTION_LEFT:
        // Handle action left
        break;
      case TYPE_ACTION_RIGHT:
        // Handle action right
        break;
      case TYPE_ACTION_ROTATE:
        // Handle action rotate
        break;
      case TYPE_ACTION_MOVE:
        // Handle action move
        break;
      case LPAREN:
        // Handle left parenthesis
        tokenStack.push(token);
        break;
      case RPAREN:
        // Handle right parenthesis
        nmath::Token* topToken;
        while(tokenStack.size() > 0) {
          topToken = tokenStack.top();
          tokenStack.pop();
          if (topToken->type == LPAREN) {
            break;
          }
          // Process the token
          tempList.push_back(topToken);
        }

        // check if there is a function or a geometric object right before the left parenthesis
        topToken = tokenStack.top();
        if (topToken->type == TYPE_CONE) {
          std::cout << "Temp list size: " << tempList.size() << std::endl;
          // Process the function or object
          float height = nmath::parseDouble<float>(tempList[4]->text, 0, tempList[4]->textLength, &errorCode);
          if (errorCode != NMATH_NO_ERROR) {
            std::cerr << "Error parsing height: " << errorCode << std::endl;
            return;
          }
          float radius = nmath::parseDouble<float>(tempList[3]->text, 0, tempList[3]->textLength, &errorCode);
          if (errorCode != NMATH_NO_ERROR) {
            std::cerr << "Error parsing radius: " << errorCode << std::endl;
            return;
          }
          float capRadius = nmath::parseDouble<float>(tempList[2]->text, 0, tempList[2]->textLength, &errorCode);
          if (errorCode != NMATH_NO_ERROR) {
            std::cerr << "Error parsing cap radius: " << errorCode << std::endl;
            return;
          }
          int stacks = nmath::parseInteger<int>(tempList[1]->text, 0, tempList[1]->textLength, &errorCode);
          if (errorCode != NMATH_NO_ERROR) {
            std::cerr << "Error parsing stacks: " << errorCode << std::endl;
            std::cerr << "Element's text: " << tempList[1]->text << std::endl;
            std::cerr << "Element's text length: " << ((int)tempList[1]->textLength) << std::endl;
            return;
          }
          int capStacks = nmath::parseInteger<int>(tempList[0]->text, 0, tempList[0]->textLength, &errorCode);
          if (errorCode != NMATH_NO_ERROR) {
            std::cerr << "Error parsing cap stacks: " << errorCode << std::endl;
            return;
          }
          std::cout << "Height: " << height << std::endl;
          std::cout << "Radius: " << radius << std::endl;
          std::cout << "Cap Radius: " << capRadius << std::endl;
          std::cout << "Stacks: " << stacks << std::endl;
          std::cout << "Cap Stacks: " << capStacks << std::endl;

          if (height <= 0.0f || capStacks <= 0) {
            std::cerr << "Cone parameter is invalid" << std::endl;
            errorCode = ERROR_INVALID_PARAMETER;
            return;
          }
          
          VboObject* aCone = createCone(height, radius, capRadius, stacks, capStacks, color[0], color[1], color[2], color[3]);
          if (aCone != nullptr) {
            std::cout << "Cone created successfully." << std::endl;
            aCone->setupArrayAttributes();
            results.push_back(aCone);
          } else {
            std::cerr << "Error creating cone object." << std::endl;
          }
          tempList.clear();
        }
        break;
      case COMMA:
        // Handle comma
        break;
      case SEMI:
        // Handle semicolon
        break;
      case NUMBER:
        // Handle number
        tokenStack.push(token);
        break;
      default:
        std::cerr << "Unknown token type: " << token->type << std::endl;
    }
    currentIndex++;
  }
  std::cout << "[parseCommand] Out of while" << std::endl;
}

VboObject* CommandHandler::createCone(float height, float radius, float capRadius, int stacks, int capStacks, 
    float red, float green, float blue, float alpha) {
  
  // Create a rounded cone mesh
  Cone cone(height, radius, capRadius, stacks, capStacks);
  std::vector<float> vertices;
  std::vector<unsigned short> indices;

  // Generate the cone mesh
  int vertexCount = cone.generateMesh(vertices, indices);

  std::cout << "Done generating mesh with vertex count = " << vertexCount << std::endl;

  // Create a color array for the vertices
  float * colors = new float[vertexCount * 4];
  int k;
  for (int i = 0; i < vertexCount; ++i) {
    k = i * 4;
    colors[k] = red;
    colors[k + 1] = green;
    colors[k + 2] = blue;
    colors[k + 3] = alpha;
  }
  std::cout << "Done assigning color." << std::endl;

  // Create a VboObject for the cone
  VboObject* vboObject = new VboObject(locations, GL_TRIANGLES);
  std::cout << "Done initializing VboObject." << std::endl;
  vboObject->initialize(vertices.data(), vertices.size(), vertexCount, colors, indices.data(), indices.size(), 3);

  delete[] colors;
  std::cout << "Done Creating Cone." << std::endl;
  return vboObject;
}

int CommandHandler::createMesh(const vector<nmath::Token *> &tokens, 
  const float *values, float epsilon, std::vector<VboObject*> &results) {

  errorCode = function.parse(tokens, &parser);
  if (errorCode != NMATH_NO_ERROR) {
    std::cerr << "Error parsing function: " << errorCode << std::endl;
    return errorCode;
  }

  std::vector<nmath::ImageData<float>*> spaces = function.getSpace(values, epsilon, true, false);
  errorCode = function.getErrorCode();
  if(errorCode != NMATH_NO_ERROR) {
    std::cerr << "Test failed! Cannot parse the expression with error code " << errorCode << std::endl;
    for(auto i=0; i< spaces.size(); i++) {
      delete spaces[i];
    }
    return errorCode;
  }
  
  VboObject* vboObject;
  unsigned int indexLength;

  for(auto i=0; i< spaces.size(); i++) {
    nmath::ImageData<float>* mesh = spaces[i];
    unsigned short* triangleTripIndices = mesh->generateIndices(indexLength);
    const float* vertices = mesh->getData();
    // Get the size of the vertices array
    int dataSize = mesh->vertexListSize();
    int vertexCount = mesh->getVertexCount();
    int stride = mesh->getDimension();
    int normalOffset = mesh->getNormalOffset();

    float* colors = new float[vertexCount * 4];
    for(int i=0; i< vertexCount; i++) {
      colors[i * 4] = 0.3f;
      colors[i * 4 + 1] = 0.5f;
      colors[i * 4 + 2] = 0.2f;
      colors[i * 4 + 3] = 1.0f;
    }
  
    // For debugging
    // std::cout << "vertexCount: " << vertexCount << std::endl;
    // std::cout << "dataSize: " << dataSize << std::endl;
    // std::cout << "indexLength: " << indexLength << std::endl;
    // std::cout << "normalOffset: " << normalOffset << std::endl;

    vboObject = new VboObject(locations, GL_TRIANGLE_STRIP);
    vboObject->initialize(vertices, dataSize, vertexCount, colors, triangleTripIndices, indexLength, normalOffset);
    results.push_back(vboObject);

    delete[] triangleTripIndices;
    delete[] colors;
  }

  return 0;
}
