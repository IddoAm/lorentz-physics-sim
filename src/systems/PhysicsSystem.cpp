// systems/PhysicsSystem.cpp
#include "PhysicsSystem.hpp"
#include "components/Particle.hpp"
#include "components/ElectricField.hpp"
#include "components/MagneticField.hpp"
#include <glm/glm.hpp>

PhysicsSystem::PhysicsSystem(entt::registry& registry)
    : m_registry(registry)
{
}

void PhysicsSystem::update(float dt) {
    applyLorentzForce(dt);
}

void PhysicsSystem::applyLorentzForce(float dt) {
    auto particles = m_registry.view<Particle>();
    auto eFields = m_registry.view<ElectricField>();
    auto bFields = m_registry.view<MagneticField>();

    for (auto&& [pEntity, particle] : particles.each()) {
        glm::vec3 force = glm::vec3(0.f);

        // accumulate electric forces
        for (auto&& [fEntity, field] : eFields.each()) {
            if (field.region.contains(particle.position.x, particle.position.y))
                force += particle.charge * field.E;
        }

        // accumulate magnetic forces
        for (auto&& [fEntity, field] : bFields.each()) {
            if (field.region.contains(particle.position.x, particle.position.y))
                force += particle.charge * glm::cross(particle.velocity, field.B);
        }

        // F = ma → a = F/m
        glm::vec3 acceleration = force / particle.mass;

        // Euler integration
        particle.velocity += acceleration * dt;
        particle.position += particle.velocity * dt;
    }
}