#ifndef _VBO_H_
#define _VBO_H_

#include <iostream>
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
        GLuint positionColorBufferId;
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
        glm::mat4 rotationMatrix;
        glm::mat4 translationMatrix;
        glm::mat4 modelMatrix;
        GLuint drawType;

    public:
        VBO(const float* bufferData, unsigned int _verticeCount, 
                const unsigned short* indices, unsigned int _indexCount, 
                ShaderVarLocation locations, bool _hasNormal) {

            this->rotationMatrix = glm::mat4(1.0f);
            this->translationMatrix = glm::mat4(1.0f);
            this->modelMatrix = glm::mat4(1.0f);

            this->positionLocation = locations.positionLocation;
            this->normalLocation = locations.normalLocation;
            this->colorLocation = locations.colorLocation;
            this->hasNormal = _hasNormal;
            this->verticeCount = _verticeCount;
            
            glGenVertexArrays(1, &vaoId);
            glGenBuffers(1, &positionColorBufferId);

            positionOffset = 0;
            normalOffset = -1;
            colorOffset = 3 * sizeof(float);

            int floatStride = 6;
            if (hasNormal) {
                floatStride = 9;
                normalOffset = 3 * sizeof(float);
                colorOffset = 6 * sizeof(float);
            }
            strideInBytes = floatStride * sizeof(float);
            glBindVertexArray(vaoId);
            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferId);
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

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        virtual ~VBO() {
            release();
        }

        void release() {
            glDeleteVertexArrays(1, &vaoId);
            glDeleteBuffers(1, &positionColorBufferId);
            glDeleteBuffers(1, &elementBufferId);
            std::cout << "Done releasing VBO" << std::endl;
        }

        void applyRotation(float xRadian, const glm::vec3 &xAxis, float yRadian, const glm::vec3 &yAxis, float zRadian, const glm::vec3 &zAxis) {
            const float zeroFloat = 0.0f;
            glm::mat4 rotMat = glm::mat4(1.0f);
            if (xRadian != zeroFloat)
                rotMat = glm::rotate(rotMat, xRadian, xAxis);
            if (yRadian != zeroFloat)
                rotMat = glm::rotate(rotMat, yRadian, yAxis);
            if (zRadian != zeroFloat)
                rotMat = glm::rotate(rotMat, zRadian, zAxis);
            
            modelMatrix = translationMatrix * rotMat;
        }

        void render(const ShaderVarLocation &shaderVarLocation) {

            glUniform1f(shaderVarLocation.useNormalID, hasNormal ? 1.0f : 0.0f);
            glUniformMatrix4fv(shaderVarLocation.modelMatrixId, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glBindVertexArray(this->vaoId);
            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferId);
            glEnableVertexAttribArray(positionLocation);
            glVertexAttribPointer(positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        strideInBytes,
                        reinterpret_cast<void*>(0));

            if (normalOffset > 0) {
                glEnableVertexAttribArray(normalLocation);
                glVertexAttribPointer(normalLocation, //Attribute index
                            3,  //Number of component per normal vector
                            GL_FLOAT,
                            GL_FALSE,
                            strideInBytes,
                            reinterpret_cast<void*>(normalOffset));
            }

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

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
};

#endif