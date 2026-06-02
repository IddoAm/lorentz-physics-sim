#include "Application.hpp"
#include <imgui.h>
#include <imgui-SFML.h>

Application::Application()
    : m_window(sf::VideoMode({ 1280, 720 }), "Lorentz Sim")
    , m_physics(m_registry)
    , m_render(m_registry, m_window)
    , m_inspector()
    , m_interaction(m_registry, m_inspector)
    , m_toolbar(m_registry, [this](SimulationMode mode) {
    mode == SimulationMode::Running ? startSimulation() : stopSimulation();
        })
{
    m_window.setFramerateLimit(60);
    ImGui::SFML::Init(m_window);
}

Application::~Application() {
    ImGui::SFML::Shutdown();
}

void Application::run() {
    while (m_running) {
        float dt = m_clock.restart().asSeconds();
        handleEvents();
        update(dt);
        render();
    }
}

void Application::handleEvents() {
    while (const auto event = m_window.pollEvent()) {
        ImGui::SFML::ProcessEvent(m_window, *event);

        if (event->is<sf::Event::Closed>())
            m_running = false;

        if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            if (key->code == sf::Keyboard::Key::Space)
                m_mode == SimulationMode::Running ? stopSimulation() : startSimulation();

        m_interaction.handleEvent(*event, m_window, m_mode);
    }
}

void Application::update(float dt) {
    ImGui::SFML::Update(m_window, m_clock.restart());

    if (m_mode == SimulationMode::Running)
        m_physics.update(dt);
    else if(m_mode == SimulationMode::Editing)
        m_interaction.update(m_window);

    m_toolbar.draw(m_mode);
    m_inspector.draw(m_registry);
}

void Application::render() {
    m_window.clear(sf::Color::Black);
    m_render.draw();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void Application::startSimulation() {
    m_snapshot = {};

    for (auto&& [e, p] : m_registry.view<Particle>().each())
        m_snapshot.particles.push_back({ e, p });

    for (auto&& [e, f] : m_registry.view<ElectricField>().each())
        m_snapshot.eFields.push_back({ e, f });

    for (auto&& [e, f] : m_registry.view<MagneticField>().each())
        m_snapshot.bFields.push_back({ e, f });

	m_interaction.onModeChange(SimulationMode::Running);
    m_mode = SimulationMode::Running;
}

void Application::stopSimulation() {
    m_registry.clear();

    for (auto& entry : m_snapshot.particles)
        m_registry.emplace<Particle>(m_registry.create(), entry.p);

    for (auto& entry : m_snapshot.eFields)
        m_registry.emplace<ElectricField>(m_registry.create(), entry.f);

    for (auto& entry : m_snapshot.bFields)
        m_registry.emplace<MagneticField>(m_registry.create(), entry.f);

    m_interaction.onModeChange(SimulationMode::Editing);
    m_mode = SimulationMode::Editing;
}