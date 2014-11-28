#ifndef _STATICOBJECT_H
#define _STATICOBJECT_H

#include <GL\glew.h>
#include <GL\GL.h>
#include "BattleObject.h"

class StaticObject: public BattleObject {
	private:
		GLenum mPrimitiveType;
		GLuint mVertexBufferId;

	public:
		StaticObject();
		~StaticObject();

		GLenum getPrimityType() { return mPrimitiveType; }
		void setPrimityType(GLenum ptype) { mPrimitiveType = ptype; }
		GLuint getVertexBufferId() { return mVertexBufferId; }
		void setVertexBufferId(GLuint bid) { mVertexBufferId = bid; }

		int load(float *vertices, int size, int sizePerVertex, int posOffs, int colorOffs, int normalOffs);
		void render();
};

#endif

