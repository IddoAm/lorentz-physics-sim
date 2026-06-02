#pragma once

#include <glm/glm.hpp>
#include "math/Rect.hpp"

struct MagneticField {
    glm::vec3 B;
    Rect      region;
};