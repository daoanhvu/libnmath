#ifndef _GAMECONTROLLER_H
#define _GAMECONTROLLER_H

#include "BattleObject.h"
#include "gamemodel.h"
#include "gameview.h"

namespace TankGame{
	class GameController {
		private:
			GameModel* mModel;
			GameView* mView;

		public:
			GameController(GameModel *model, GameView *view);
			~GameController();

			int init();
			void start();
			void stop();

			void onSelectObject(BattleObject *obj);
			void onMoveObject(BattleObject *obj, float x, float y, float z);

			void pauseGame();
	};
}

#endif