#ifndef _BattleObject_H
#define _BattleObject_H

namespace TankGame {
	class BattleObject {
		protected:
			float mX;
			float mY;
			float mZ;
			int mHP;

		public:
			BattleObject();
			virtual ~BattleObject() {};

			virtual void setLocation(float x, float y, float z)	{ mX = x; mY = y; mZ = z; }
			virtual void setHP(int hp) { mHP = hp; }
			virtual int getHP() { return mHP; }
			virtual int load(float *vertices, int size)	{ return 0; }
	};
}

#endif