#include "gamecontroller.h"

using namespace TankGame;

GameController::GameController(GameModel *model, GameView *view): mModel(model), mView(view) {

}

GameController::~GameController() {

}

int GameController::init() {
	return mView->init(3, 1);
}

void GameController::start() {
	mView->start();
}

void GameController::stop() {
	mView->close();
}