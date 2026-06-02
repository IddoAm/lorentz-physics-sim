// ui/Inspector.cpp
#include "Inspector.hpp"
#include "components/Particle.hpp"
#include "components/ElectricField.hpp"
#include "components/MagneticField.hpp"
#include <imgui.h>

void Inspector::draw(entt::registry& registry) {
    ImGui::Begin("Inspector");

    if (m_selected == entt::null || !registry.valid(m_selected)) {
        ImGui::Text("Nothing selected");
        ImGui::End();
        return;
    }

    drawParticle(registry);
    drawElectricField(registry);
    drawMagneticField(registry);

    ImGui::Separator();
    if (ImGui::Button("Delete")) {
        registry.destroy(m_selected);
        deselect();
    }

    ImGui::End();
}

void Inspector::drawParticle(entt::registry& registry) {
    if (!registry.all_of<Particle>(m_selected)) return;
    auto& p = registry.get<Particle>(m_selected);

    if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Position", &p.position.x, 0.1f);
        ImGui::DragFloat3("Velocity", &p.velocity.x, 0.1f);
        ImGui::DragFloat("Charge", &p.charge, 0.1f);
        ImGui::DragFloat("Mass", &p.mass, 0.1f, 0.01f);  // min mass 0.01
    }
}

void Inspector::drawElectricField(entt::registry& registry) {
    if (!registry.all_of<ElectricField>(m_selected)) return;
    auto& f = registry.get<ElectricField>(m_selected);

    if (ImGui::CollapsingHeader("Electric Field", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("E", &f.E.x, 0.1f);
        ImGui::DragFloat2("Position", &f.region.x, 1.f);
        ImGui::DragFloat2("Size", &f.region.w, 1.f, 10.f);
    }
}

void Inspector::drawMagneticField(entt::registry& registry) {
    if (!registry.all_of<MagneticField>(m_selected)) return;
    auto& f = registry.get<MagneticField>(m_selected);

    if (ImGui::CollapsingHeader("Magnetic Field", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("B", &f.B.x, 0.1f);
        ImGui::DragFloat2("Position", &f.region.x, 1.f);
        ImGui::DragFloat2("Size", &f.region.w, 1.f, 10.f);
    }
}