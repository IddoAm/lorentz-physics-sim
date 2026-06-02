// core/Application.hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include "systems/PhysicsSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "ui/Inspector.hpp"
#include "ui/Toolbar.hpp"
#include "ui/InteractionHandler.hpp"
#include "SimulationMode.hpp"

struct SceneSnapshot {
    struct ParticleEntry { entt::entity e; Particle p; };
    struct EFieldEntry { entt::entity e; ElectricField f; };
    struct BFieldEntry { entt::entity e; MagneticField f; };

    std::vector<ParticleEntry> particles;
    std::vector<EFieldEntry>   eFields;
    std::vector<BFieldEntry>   bFields;
};

class Application {
public:
    Application();
    ~Application();
    void run();

private:
    void startSimulation();
    void stopSimulation();

    // loop
    void handleEvents();
    void update(float dt);
    void render();

    // window
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    bool             m_running = true;

    // world
    entt::registry   m_registry;
    PhysicsSystem    m_physics;
    RenderSystem     m_render;
    Inspector        m_inspector;
	Toolbar 		 m_toolbar;
	InteractionHandler m_interaction;

    SimulationMode        m_mode = SimulationMode::Editing;
    SceneSnapshot m_snapshot;
};