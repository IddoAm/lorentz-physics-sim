// ui/Toolbar.hpp
#pragma once
#include "core/SimulationMode.hpp"
#include <entt/entt.hpp>
#include <functional>

class Toolbar {
public:
    Toolbar(entt::registry& registry, std::function<void(SimulationMode)> onModeChange);
    void draw(SimulationMode currentMode);

private:
    entt::registry& m_registry;
    std::function<void(SimulationMode)> m_onModeChange;
};