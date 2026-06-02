#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float charge;
    float mass;
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Lorentz Sim");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    entt::registry registry;

    // Spawn a test particle
    auto entity = registry.create();
    registry.emplace<Particle>(entity,
        glm::vec3(400.f, 300.f, 0.f),  // position
        glm::vec3(1.f, 0.5f, 0.f),     // velocity
        1.f,                             // charge
        1.f                              // mass
    );

    sf::Clock clock;
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt);

        // Inspector
        ImGui::Begin("Particle Inspector");
        auto& p = registry.get<Particle>(entity);
        ImGui::DragFloat3("Position", &p.position.x, 0.1f);
        ImGui::DragFloat3("Velocity", &p.velocity.x, 0.1f);
        ImGui::DragFloat("Charge", &p.charge, 0.1f);
        ImGui::DragFloat("Mass", &p.mass, 0.1f);
        ImGui::End();

        window.clear(sf::Color::Black);

        // Draw particle as a circle
        sf::CircleShape circle(6.f);
        circle.setFillColor(p.charge > 0 ? sf::Color::Red : sf::Color::Cyan);
        circle.setOrigin({ 6.f, 6.f });
        circle.setPosition({ p.position.x, p.position.y });
        window.draw(circle);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}