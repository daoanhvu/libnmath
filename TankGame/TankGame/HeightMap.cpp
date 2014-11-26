#if WIN32
#include <Windows.h>
#else
#define NULL ((void*)0)
#endif

#include "HeightMap.h"

using namespace TankGame;


HeightMap::HeightMap():mMap(NULL), mWidth(0), mHeight(0) {
}


HeightMap::~HeightMap() {
	if (mMap != NULL) {
		delete[] mMap;
		mWidth = 0;
		mHeight = 0;
	}
}

void HeightMap::loadRawFile(const char *filename) {

}

int HeightMap::height(int x, int y) {
	return mMap[x*mHeight + y];
}

void HeightMap::render() {

}