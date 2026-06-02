#pragma once
#include <entt/entt.hpp>

class PhysicsSystem {
public:
    explicit PhysicsSystem(entt::registry& registry);
    void update(float dt);

private:
    void applyLorentzForce(float dt);

    entt::registry& m_registry;
};