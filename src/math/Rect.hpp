#pragma once
#pragma once

struct Rect {
    float x, y;       // top-left (or origin)
    float w, h;       // width, height

    bool contains(float px, float py) const {
        return px >= x && px <= x + w &&
            py >= y && py <= y + h;
    }
};