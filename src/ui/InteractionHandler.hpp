#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <array>
#include "core/SimulationMode.hpp"
#include "components/Particle.hpp"
#include "components/ElectricField.hpp"
#include "components/MagneticField.hpp"
#include "math/Rect.hpp"

class Inspector;

class InteractionHandler {
public:
    InteractionHandler(entt::registry& registry, Inspector& inspector);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window, SimulationMode mode);

    enum class Handle {
        None,
        TopLeft, Top, TopRight,
        Left, Right,
        BottomLeft, Bottom, BottomRight
    };

    // how close (in world units) the cursor must be to grab an edge/corner
    static constexpr float kHandleSize = 6.f;
    static constexpr float kMinFieldSize = 10.f;

    // helper the renderer can use to draw the 8 handles of a selected field
    static std::array<sf::Vector2f, 8> handlePositions(const Rect& r);

private:
    void trySelect(sf::Vector2f worldPos);

    void spawnParticle(sf::Vector2f worldPos);
    void spawnElectricField(sf::Vector2f worldPos);
    void spawnMagneticField(sf::Vector2f worldPos);

    void beginDrag(sf::Vector2f worldPos);
    void updateDrag(sf::Vector2f worldPos);
    void endDrag();

    Rect* getRegion(entt::entity e);   // region ptr if entity is a field, else nullptr

    sf::Vector2f toWorldPos(sf::RenderWindow& window);

    entt::registry& m_registry;
    Inspector& m_inspector;

    entt::entity m_dragging = entt::null;
    bool         m_isDragging = false;
    sf::Vector2f m_dragOffset{};
    Handle       m_resizeHandle = Handle::None;
};