#ifndef _VBO_H_
#define _VBO_H_

#include <vector>
#include <GL/glew.h>
#include <shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class VBO {
    private:
        GLuint vaoId;
        // position, color bufferId
        GLuint positionColorBufferIds[2];
        GLuint elementBufferId;
        unsigned int indexCount;
        // This is THE number of vertice in the VBO, NOT the number of float in [vertices]
        int verticeCount;
        GLsizeiptr strideInBytes;
        GLintptr positionOffset;
        GLintptr normalOffset;
        GLintptr colorOffset;
        // shader's variables locations
        GLuint positionLocation;
        GLuint colorLocation;
        bool hasNormal;
        GLuint normalLocation;
        glm::mat4 modelMatrix;
        GLuint drawType;

    public:
        VBO(const float* bufferData, unsigned int _verticeCount, 
                const unsigned short* indices, unsigned int _indexCount, 
                ShaderVarLocation locations, bool _hasNormal) {

            this->positionLocation = locations.positionLocation;
            this->colorLocation = locations.colorLocation;
            this->normalLocation = locations.normalLocation;
            this->hasNormal = _hasNormal;

            this->verticeCount = _verticeCount;
            
            glGenVertexArrays(1, &vaoId);
            glBindVertexArray(vaoId);

            positionOffset = 0;
            normalOffset = 0;
            colorOffset = 3 * sizeof(float);

            int floatStride = 6;
            if (hasNormal) {
                floatStride = 9;
                normalOffset = 3 * sizeof(float);
                colorOffset = 6 * sizeof(float);
            }
            strideInBytes = floatStride * sizeof(float);

            glGenBuffers(2, positionColorBufferIds);
            std::cout << "GOT HERE 2 " << std::endl;
            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferIds[0]);
            std::cout << "GOT HERE 3 " << std::endl;
            glBufferData(GL_ARRAY_BUFFER, verticeCount * strideInBytes, bufferData, GL_STATIC_DRAW);
            std::cout << "Done preparing array buffer data " << std::endl;

            this->indexCount = 0;
            if (indices != nullptr) {
                this->indexCount = _indexCount;
                glGenBuffers(1, &elementBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);
                GLsizeiptr indicesSizeInByte = indexCount * sizeof(unsigned short);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSizeInByte, indices , GL_STATIC_DRAW);
            }

            glEnableVertexAttribArray(positionLocation);
	        glVertexAttribPointer(positionLocation, //Attribute index
				3,  //Number of component per this attribute of vertex
				GL_FLOAT,
				GL_FALSE,
				strideInBytes,
				//(void*)(uintptr_t)0); //TODO:Hard code here!!!!!
				reinterpret_cast<void*>(positionOffset));

            if (hasNormal) {
                glEnableVertexAttribArray(normalLocation);
                glVertexAttribPointer(normalLocation, //Attribute index
                        3,  //Number of component per vertex
                        GL_FLOAT,
                        GL_FALSE,
                        strideInBytes,
                        // (void*)(uintptr_t)normalOffset);
                        reinterpret_cast<void*>(normalOffset));
            }

            glEnableVertexAttribArray(colorLocation);
		    glVertexAttribPointer(colorLocation, //Attribute index
			3,  //Number of component per vertex
			GL_FLOAT,
			GL_FALSE,
			strideInBytes,
			// (void*)(uintptr_t)colorOffset);
			reinterpret_cast<void*>(colorOffset));
        }

        virtual ~VBO() {
            release();
        }

        void release() {
            glDeleteVertexArrays(1, &vaoId);
            glDeleteBuffers(2, positionColorBufferIds);
            // delete element data
            glDeleteBuffers(1, &elementBufferId);
            std::cout << "Done releasing VBO" << std::endl;
        }

        void applyRotation(const glm::quat &rotQuat) {
            // Create a rotation matrix from the quaternion
            this->modelMatrix = glm::mat4_cast(rotQuat);
        }

        void render(const ShaderVarLocation &shaderVarLocation, const glm::mat4 &projectionMatrix, 
				const glm::mat4 &viewMatrix) {

            glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

            glUniform1i(shaderVarLocation.useNormalLocation, (normalOffset>=0)?1:0);
            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(shaderVarLocation.mvpMatrixId, 1, GL_FALSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(shaderVarLocation.modelMatrixId, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shaderVarLocation.viewMatrixId, 1, GL_FALSE, glm::value_ptr(viewMatrix));

            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferIds[0]);
            glEnableVertexAttribArray(positionLocation);
            glVertexAttribPointer(positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        strideInBytes,
                        reinterpret_cast<void*>(0));

            glEnableVertexAttribArray(normalLocation);
            glVertexAttribPointer(normalLocation, //Attribute index
                        3,  //Number of component per normal vector
                        GL_FLOAT,
                        GL_FALSE,
                        strideInBytes,
                        reinterpret_cast<void*>(normalOffset));

            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferIds[1]);
            glEnableVertexAttribArray(colorLocation);
            glVertexAttribPointer(colorLocation, //Attribute index
                    3,  //Number of component per color. We have red, green, blue and alpha
                    GL_FLOAT,
                    GL_FALSE,
                    strideInBytes,
                    reinterpret_cast<void*>(colorOffset));
            // Bind buffer for indices
            if (this->indexCount > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);
                glDrawElements(
                        GL_TRIANGLE_STRIP,  // mode
                        indexCount,         // count
                        GL_UNSIGNED_SHORT,  // type
                        NULL);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            } else {
                glDrawArrays(GL_LINES, 0, verticeCount);
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDisableVertexAttribArray(positionLocation);
            glDisableVertexAttribArray(normalLocation);
            glDisableVertexAttribArray(colorLocation);
        }
};

#endif