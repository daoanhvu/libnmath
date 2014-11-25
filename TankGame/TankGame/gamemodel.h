#ifndef _GAMEMODEL_H
#define _GAMEMODEL_H

#include <GL\glew.h>

namespace TankGame {
	class GameModel {
		private:
			

		public:
			GameModel();
			~GameModel();

			void init();
			int loadFromFile(const char *filename);
			int saveToFile(const char *filename);
	};
}

#endif