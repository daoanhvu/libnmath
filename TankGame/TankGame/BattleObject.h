#ifndef _BattleObject_H
#define _BattleObject_H

	class BattleObject {
		protected:
			float mX;
			float mY;
			float mZ;
			int mHP;

		public:
			BattleObject();
			virtual ~BattleObject() {}
			virtual void release()	{}

			virtual void setLocation(float x, float y, float z)	{ mX = x; mY = y; mZ = z; }
			virtual void setHP(int hp) { mHP = hp; }
			virtual int getHP() { return mHP; }
			virtual int load(float *vertices, int size, int sizePerVertex, int posOffs, int colorOffs, int normalOffs)	{ return 0; }
	};


#endif