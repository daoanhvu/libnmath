#include "StaticObject.h"


StaticObject::StaticObject() {
}


StaticObject::~StaticObject() {
}

/*
	This method must be called after the object get a vertex buffer id and have a primitive type.
*/
int StaticObject::load(short *indices, int indexSize, float *vertices, int size, int sizePerVertex, int posOffs, int colorOffs, int normalOffs) {
	glGenBuffers(2, mBufferId);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * 2, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, size * 4, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return 0;
}

void StaticObject::render() {

}