#ifndef _GAMEVIEW_H
#define _GAMEVIEW_H

#include <Windows.h>
#include <GLFW\glfw3.h>
#include "gamemodel.h"

namespace TankGame {
	class GameView {
		private:
			GLFWwindow *mWindow;
			GLuint mProgramId;
			GLuint mMVPMatrixId;
			GameModel *mModel;
			HANDLE mThreadHandle;
			unsigned int mThreadID;
			volatile bool isRunning;

		public:
			GameView(GameModel *model);
			~GameView();

			int init();
			void start();
			void setCamera();
			void close();

			void runThread();
			static void startThread(void *param);

			void onMouseClick(int x, int y, int button);
			void onMouseDown(int x, int y, int button);
			void onMouseUp(int x, int y, int button);
			void onMouseMove(int x, int y);

	};
}

#endif