#include "StaticObject.h"


StaticObject::StaticObject() {
}


StaticObject::~StaticObject() {
}

/*
	This method must be called after the object get a vertex buffer id and have a primitive type.
*/
int StaticObject::load(float *vertices, int size, int sizePerVertex, int posOffs, int colorOffs, int normalOffs) {
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, size * 4, vertices, GL_STATIC_DRAW);
	return 0;
}

void StaticObject::render() {

}