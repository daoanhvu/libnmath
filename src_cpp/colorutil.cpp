#include <colorutil.h>


namespace nmath {
    void intColorToFloats(int color, float colors[4]) {
        const float MAX_COLOR_VALUE = 255.0f;
        colors[0] = (color & 0xFF) / MAX_COLOR_VALUE;
        colors[1] = ((color >> 8) & 0xFF) / MAX_COLOR_VALUE;
        colors[2] = ((color >> 16) & 0xFF) / MAX_COLOR_VALUE;
        colors[3] = ((color >> 24) & 0xFF) / MAX_COLOR_VALUE;
    }
}