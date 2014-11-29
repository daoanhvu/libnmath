#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

#include "tankgame.h"
#include "gamecontroller.h"
#include "gameview.h"
#include "gamemodel.h"

using namespace glm;
using namespace TankGame;

int main(int argc, char* args[]) {

	GameModel gameModel;
	GameView gameView(&gameModel);
	GameController gameController(&gameModel, &gameView);

	if (gameController.init() == 0) {
		gameController.start();
		gameController.stop();
	}

	return 0;
}
