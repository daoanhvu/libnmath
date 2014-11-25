#include "gamecontroller.h"

using namespace TankGame;

GameController::GameController(GameModel *model, GameView *view): mModel(model), mView(view) {

}

GameController::~GameController() {

}

int GameController::init() {
	return mView->init();
}

void GameController::start() {
	mView->start();
}

void GameController::stop() {
	mView->close();
}