// ui/Toolbar.cpp
#include "Toolbar.hpp"
#include "components/Particle.hpp"
#include "components/ElectricField.hpp"
#include "components/MagneticField.hpp"
#include <imgui.h>

Toolbar::Toolbar(entt::registry& registry, std::function<void(SimulationMode)> onModeChange)
    : m_registry(registry)
    , m_onModeChange(onModeChange)
{
}

void Toolbar::draw(SimulationMode currentMode) {
    ImGui::Begin("Toolbar");

    // spawn buttons — only in edit mode
    if (currentMode == SimulationMode::Editing) {
        if (ImGui::Button("Add Particle")) {
            auto entity = m_registry.create();
            m_registry.emplace<Particle>(entity,
                glm::vec3(400.f, 300.f, 0.f),
                glm::vec3(0.f),
                1.f, 1.f
            );
        }
        if (ImGui::Button("Add Electric Field")) {
            auto entity = m_registry.create();
            m_registry.emplace<ElectricField>(entity,
                glm::vec3(1.f, 0.f, 0.f),
                Rect{ 300.f, 200.f, 200.f, 200.f }
            );
        }
        if (ImGui::Button("Add Magnetic Field")) {
            auto entity = m_registry.create();
            m_registry.emplace<MagneticField>(entity,
                glm::vec3(0.f, 0.f, 1.f),
                Rect{ 300.f, 200.f, 200.f, 200.f }
            );
        }

        ImGui::Separator();
    }

    // simulation controls
    if (currentMode == SimulationMode::Editing) {
        if (ImGui::Button("Play"))
            m_onModeChange(SimulationMode::Running);
    }
    else {
        if (ImGui::Button("Stop"))
            m_onModeChange(SimulationMode::Editing);
    }

    ImGui::End();
}