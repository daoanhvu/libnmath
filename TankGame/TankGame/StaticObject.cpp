#include "StaticObject.h"


StaticObject::StaticObject() : mBufferId(0), mBufferCount(0) {
}


StaticObject::~StaticObject() {
	release();
}

void StaticObject::release() {
	if (mBufferId != 0){
		glDeleteBuffers(mBufferCount, mBufferId);
		delete[] mBufferId;
		mBufferId = (GLuint*)0;
		mBufferCount = 0;
	}
}

/*
	This method must be called after the object get a vertex buffer id and have a primitive type.
*/
int StaticObject::load(short *indices, int indexSize, float *vertices, int vCount, int sizePerVertex, 
				int posSize, int colorSize, int normalSize) {

	GLsizei dataSize = vCount * sizePerVertex;
	mBufferCount = 2;
	mBufferId = new GLuint[mBufferCount];
	mVertexCount = vCount;

	glGenBuffers(mBufferCount, mBufferId);

	mStride = 4 * (posSize + colorSize + normalSize);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * 2, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, dataSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	return 0;
}

void StaticObject::render(GLuint positionLoc, GLuint colorLoc) {

	glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, mStride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, mStride, (void*)3);

	glDrawArrays(mPrimitiveType, 0, mVertexCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}