#pragma once
#include "BattleObject.h"
class Tank : public BattleObject {
private:
	float velocityX;
	float velocityY;
	float velocityZ;

public:
	Tank();
	~Tank();
};

