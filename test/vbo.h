#ifndef _VBO_H_
#define _VBO_H_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <imagedata.hpp>

const int stride3FloatInByte = 3 * sizeof(float);
const int stride4FloatInByte = 4 * sizeof(float);
const int stride10FloatInByte = 10 * sizeof(float);
const int positionOffset = 0;
const int normalOffset = 3 * sizeof(float);
const int colorOffset = 6 * sizeof(float);

class VBO {
    private:
        GLuint vaoId;
        // position, color bufferId
        GLuint positionColorBufferIds[2];
        GLuint elementBufferId;
        float *vertices;
        short *indices;
        int indexCount;
        // This is THE number of vertice in the VBO, NOT the number of float in [vertices]
        int verticeCount;
        GLsizei strideInByte;

        // shader's variables locations
        GLuint positionLocation;
        GLuint colorLocation;
        GLuint normalLocation;

    public:
        VBO(nmath::ImageData<float> *imageData, GLuint posLocation, GLuint cLocation, GLuint norLocation) {
            this->positionLocation = posLocation;
            this->colorLocation = cLocation;
            this->normalLocation = norLocation;

            // It needs 3 floats for position, 3 floats for normal and 4 floats for color
            int floatStride = 10;
            //TODO: generate vertices from ImageData object
            float *verticesBufferData = new float[imageData->getVertexCount() * floatStride];
            imageData->copyDataWithColorTo(863, verticesBufferData);

            //Bind position, color, normal
            strideInByte = floatStride * sizeof(float);

            glGenVertexArrays(1, &vaoId);
            glBindVertexArray(vaoId);

            glGenBuffers(2, positionColorBufferIds);
            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferIds[0]);
            glBufferData(GL_ARRAY_BUFFER, verticeCount * strideInByte, verticesBufferData, GL_STATIC_DRAW);
            //Now we can release memory for vertices data
            delete[] verticesBufferData;
            verticesBufferData = nullptr;

            unsigned int indexLen;
            unsigned short *indices = imageData->generateIndices(indexLen);
            glGenBuffers(1, &elementBufferId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexLen * sizeof(unsigned short), indices , GL_STATIC_DRAW);
            delete[] indices;

            glEnableVertexAttribArray(posLocation);
	        glVertexAttribPointer(posLocation, //Attribute index
				3,  //Number of component per this attribute of vertex
				GL_FLOAT,
				GL_FALSE,
				strideInByte,
				//(void*)(uintptr_t)0); //TODO:Hard code here!!!!!
				reinterpret_cast<void*>(positionOffset));

            glEnableVertexAttribArray(normalLocation);
		    glVertexAttribPointer(normalLocation, //Attribute index
					3,  //Number of component per vertex
					GL_FLOAT,
					GL_FALSE,
					strideInByte,
					// (void*)(uintptr_t)normalOffset);
					reinterpret_cast<void*>(normalOffset));

            glEnableVertexAttribArray(colorLocation);
		    glVertexAttribPointer(colorLocation, //Attribute index
			4,  //Number of component per vertex
			GL_FLOAT,
			GL_FALSE,
			strideInByte,
			// (void*)(uintptr_t)colorOffset);
			reinterpret_cast<void*>(colorOffset));
        }

        virtual ~VBO() {
            release();
        }

        void release() {
            if (vertices != nullptr) {
                delete[] vertices;
                verticeCount = 0;
            }
            glDeleteVertexArrays(1, &vaoId);
            glDeleteBuffers(2, positionColorBufferIds);
            // delete element data
            glDeleteBuffers(1, &elementBufferId);
        }

        void render() {
            glUniform1i(shaderVarLocation.useNormalLocation, (normalOffset>=0)?1:0);
            glUniform1i(shaderVarLocation.useLightingLocation, 1);
            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(shaderVarLocation.mvpMatrixId, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(shaderVarLocation.modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);
            glUniformMatrix4fv(shaderVarLocation.viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);

            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferIds[0]);
            glEnableVertexAttribArray(positionLocation);
            glVertexAttribPointer(positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        stride10FloatInByte,
                        reinterpret_cast<void*>(0));

            glEnableVertexAttribArray(normalLocation);
            glVertexAttribPointer(normalLocation, //Attribute index
                        3,  //Number of component per normal vector
                        GL_FLOAT,
                        GL_FALSE,
                        stride10FloatInByte,
                        reinterpret_cast<void*>(normalOffset));

            glBindBuffer(GL_ARRAY_BUFFER, positionColorBufferIds[1]);
            glEnableVertexAttribArray(colorLocation);
            glVertexAttribPointer(colorLocation, //Attribute index
                    4,  //Number of component per color. We have red, green, blue and alpha
                    GL_FLOAT,
                    GL_FALSE,
                    stride10FloatInByte,
                    reinterpret_cast<void*>(4));
            // Bind buffer for indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);
            glDrawElements(
                    GL_TRIANGLE_STRIP,  // mode
                    indexCount,         // count
                    GL_UNSIGNED_SHORT,  // type
                    NULL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDisableVertexAttribArray(positionLocation);
            glDisableVertexAttribArray(normalLocation);
            glDisableVertexAttribArray(colorLocation);
        }
};

#endif