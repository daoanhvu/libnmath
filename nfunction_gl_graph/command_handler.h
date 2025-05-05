#ifndef _COMMAND_HANDLER_H_
#define _COMMAND_HANDLER_H_
#include <string>
#include <vector>

#include "common.hpp"
#include "imagedata.hpp"
#include "nlablexer.h"
#include "nfunction.hpp"
#include "SimpleCriteria.hpp"
#include "vbo_objects.h"
#include "builtin/cone.hpp"

// extended token types
#define TYPE_CONE 		0x00010005

#define TYPE_ACTION_UP 		    0x00010015
#define TYPE_ACTION_DOWN 		  0x00010016
#define TYPE_ACTION_LEFT 		  0x00010017
#define TYPE_ACTION_RIGHT 		0x00010018
#define TYPE_ACTION_ROTATE 		0x00010019
#define TYPE_ACTION_MOVE 		  0x00010020

#define ERROR_INVALID_PARAMETER 				-21

class CommandHandler {
public:
    CommandHandler() = default;
    CommandHandler(const ShaderVarLocation &locations): locations(locations) {
        // Constructor implementation
    };
    ~CommandHandler() = default;

    void setLocations(const ShaderVarLocation &locations) {
      this->locations = locations;
    }

    int getErrorCode() const {
      return errorCode;
    }
    
    int getErrorColumn() const {
      return errorColumn;
    }

    // Function to handle commands
    void handleCommand(const std::string& commandText, const float *color, std::vector<VboObject *> &results);
    // Add more member functions as needed
private:
    ShaderVarLocation locations;
    nmath::NLabLexer lexer;
    nmath::NFunction<float> function;
    nmath::NLabParser<float> parser;
    int errorCode;
    int errorColumn;
    // Private member variables and functions
    // For example, a function to parse the command string
    void parseCommand(const vector<nmath::Token*> &tokens, const float *color, std::vector<VboObject *> &results);

    VboObject* createCone(float height, float radius, float capRadius, int stacks, int capStacks,
      float red, float green, float blue, float alpha);
      
    int createMesh(const vector<nmath::Token*> &tokens, const float *values, float epsilon, std::vector<VboObject*> &results);
};

#endif