#ifndef _VBO_H_
#define _VBO_H_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <imagedata.hpp>

const int stride3FloatInByte = 3 * sizeof(float);
const int stride4FloatInByte = 4 * sizeof(float);

class VBO {
    private:
        GLuint vaoId;
        // position, color and element bufferId
        GLuint bufferIds[3];
        float *vertices;
        short *indices;
        int indexCount;
        // This is THE number of vertice in the VBO, NOT the number of float in [vertices]
        int verticeCount;
        GLsizei strideInByte;

    public:
        VBO(nmath::ImageData<float> &imageData) {
            int positionOffset = 0;
            int colorOffset = 3 * sizeof(float);
            int normalOffset = 6 * sizeof(float);	

            unsigned int *indices;
            unsigned int buff_len;
            int floatStride = 9;
            //TODO
            float *verticesBufferData = nullptr;

            //Bind position, color, normal
            strideInByte = floatStride * sizeof(float);

            glGenVertexArrays(1, &vaoId);
            glBindVertexArray(vaoId);

            glGenBuffers(3, &bufferIds);
            glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
            glBufferData(GL_ARRAY_BUFFER, verticeCount * strideInByte, verticesBufferData, GL_STATIC_DRAW);

            glEnableVertexAttribArray(location.positionLocation);
	        glVertexAttribPointer(location.positionLocation, //Attribute index
				3,  //Number of component per this attribute of vertex
				GL_FLOAT,
				GL_FALSE,
				strideInByte,
				//(void*)(uintptr_t)0); //TODO:Hard code here!!!!!
				reinterpret_cast<void*>(positionOffset));

            glEnableVertexAttribArray(location.colorLocation);
		    glVertexAttribPointer(location.colorLocation, //Attribute index
			3,  //Number of component per vertex
			GL_FLOAT,
			GL_FALSE,
			strideInByte,
			// (void*)(uintptr_t)colorOffset);
			reinterpret_cast<void*>(colorOffset));

            glEnableVertexAttribArray(location.normalLocation);
		    glVertexAttribPointer(location.normalLocation, //Attribute index
					3,  //Number of component per vertex
					GL_FLOAT,
					GL_FALSE,
					strideInByte,
					// (void*)(uintptr_t)normalOffset);
					reinterpret_cast<void*>(normalOffset));
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
            glDeleteBuffers(1, &vboId);
        }

        void render() {
            glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
            glEnableVertexAttribArray(shaderVarLocation.positionLocation);
            glVertexAttribPointer(shaderVarLocation.positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        stride3FloatInByte,
                        reinterpret_cast<void*>(0));

            glEnableVertexAttribArray(shaderVarLocation.normalLocation);
            glVertexAttribPointer(shaderVarLocation.normalLocation, //Attribute index
                        3,  //Number of component per normal vector
                        GL_FLOAT,
                        GL_FALSE,
                        stride3FloatInByte,
                        reinterpret_cast<void*>(3));

            glBindBuffer(GL_ARRAY_BUFFER, bufferIds[1]);
            glEnableVertexAttribArray(shaderVarLocation.colorLocation);
            glVertexAttribPointer(shaderVarLocation.colorLocation, //Attribute index
                    4,  //Number of component per color. We have red, green, blue and alpha
                    GL_FLOAT,
                    GL_FALSE,
                    stride4FloatInByte,
                    0);
            // Bind buffer for indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[0]);
            glDrawElements(
                    GL_TRIANGLE_TRIP,      // mode
                    indexCount,    // count
                    GL_UNSIGNED_SHORT,   // type
                    NULL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDisableVertexAttribArray(shaderVarLocation.positionLocation);
            glDisableVertexAttribArray(shaderVarLocation.normalLocation);
            glDisableVertexAttribArray(shaderVarLocation.colorLocation);
        }
};

#endif