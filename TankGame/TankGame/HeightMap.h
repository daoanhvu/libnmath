#ifndef _HEIGHTMAP_H
#define _HEIGHTMAP_H

namespace TankGame {
	class HeightMap {
		private:
			unsigned char *mMap;
			int mWidth;
			int mHeight;

		public:
			HeightMap();
			~HeightMap();

			void loadRawFile(const char *filename);
			int height(int x, int y);
			void render();
	};
}

#endif
