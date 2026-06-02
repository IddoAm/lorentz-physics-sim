#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include "math/Rect.hpp"

struct Highlight {
    bool hovered = false;
    bool selected = false;
};

class RenderSystem {
public:
    RenderSystem(entt::registry& registry, sf::RenderWindow& window);

    void draw();

private:
    void drawParticles();
    void drawFields();

    void drawField(entt::entity entity, const Rect& region, const glm::vec3& vec,
        sf::Color fill, sf::Color outline, sf::Color arrow);

    void drawFieldRegion(const Rect& region, sf::Color fill, sf::Color outline);
    void drawFieldDirection(const Rect& region, const glm::vec3& vec, sf::Color color);

    void drawArrow(sf::Vector2f from, sf::Vector2f to, sf::Color color);
    void drawOutOfPlaneSymbol(sf::Vector2f center, sf::Color color, bool outOfScreen, float radius);
    void drawLine(sf::Vector2f a, sf::Vector2f b, sf::Color color);

    void drawSelectionOutline(sf::Vector2f center, float baseRadius);
    void drawSelectionOutline(const Rect& region);

    Highlight highlightOf(entt::entity e) const;

    entt::registry& m_registry; 
    sf::RenderWindow& m_window;
};