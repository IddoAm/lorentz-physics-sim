#pragma once

#include <glm/glm.hpp>
#include "math/Rect.hpp"

struct ElectricField {
    glm::vec3 E;
    Rect      region;
};
