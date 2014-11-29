#ifndef _STATICOBJECT_H
#define _STATICOBJECT_H

#include <GL\glew.h>
#include <GL\GL.h>
#include "BattleObject.h"

class StaticObject: public BattleObject {
	private:
		GLsizei mVertexCount;
		GLenum mPrimitiveType;
		GLuint* mBufferId; //one for index buffer, one for vertex buffer
		GLsizei mBufferCount;
		GLsizei mStride;


	public:
		StaticObject();
		~StaticObject();
		void release();

		GLenum getPrimityType() { return mPrimitiveType; }
		void setPrimityType(GLenum ptype) { mPrimitiveType = ptype; }
		GLuint getBufferId(int idx) { return mBufferId[idx]; }

		int load(short *indices, int indexSize, float *vertices, int vCount, 
						int sizePerVertex, int posOffs, int colorOffs, int normalOffs);
		void render(GLuint positionLoc, GLuint colorLoc);
};

#endif

