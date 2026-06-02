// ui/Inspector.hpp
#pragma once
#include <entt/entt.hpp>

class Inspector {
public:
    void select(entt::entity entity) { m_selected = entity; }
    void deselect() { m_selected = entt::null; }
    bool hasSelection() const { return m_selected != entt::null; }
    entt::entity selected() const { return m_selected; }

    void draw(entt::registry& registry);

private:
    void drawParticle(entt::registry& registry);
    void drawElectricField(entt::registry& registry);
    void drawMagneticField(entt::registry& registry);

    entt::entity m_selected = entt::null;
};