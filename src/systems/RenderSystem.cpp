#include "RenderSystem.hpp"
#include "components/Particle.hpp"
#include "components/ElectricField.hpp"
#include "components/MagneticField.hpp"

#include <array>
#include <cmath>
#include <algorithm>

namespace {
    constexpr float kGridSpacing = 40.f;  // distance between annotations
    constexpr float kArrowLength = 20.f;  // max in-plane arrow length
    constexpr float kArrowMin = 6.f;   // keep weak components visible
    constexpr float kArrowHead = 6.f;
    constexpr float kSymbolRadius = 8.f;   // max out-of-plane ring radius
    constexpr float kSymbolMin = 3.f;
    constexpr float kZeroEps = 1e-6f;
}

RenderSystem::RenderSystem(entt::registry& registry, sf::RenderWindow& window)
    : m_registry(registry)
    , m_window(window)
{
}

void RenderSystem::draw() {
    drawFields();    // fields first so particles render on top
    drawParticles();
}

void RenderSystem::drawParticles() {
    auto view = m_registry.view<Particle>();

    const float H = static_cast<float>(m_window.getSize().y);

    sf::CircleShape circle(6.f);
    circle.setOrigin({ 6.f, 6.f });

    for (auto&& [entity, particle] : view.each()) {
        // Flip y: world is y-up, screen is y-down
        circle.setPosition({ particle.position.x, H - particle.position.y });
        circle.setFillColor(particle.charge > 0.f ? sf::Color::Red : sf::Color::Cyan);
        m_window.draw(circle);
    }
}

void RenderSystem::drawFields() {
    for (auto&& [entity, field] : m_registry.view<ElectricField>().each()) {
        drawFieldRegion(field.region,
            sf::Color(255, 255, 0, 40),
            sf::Color(255, 255, 0, 180));
        drawFieldDirection(field.region, field.E, sf::Color(255, 255, 0, 220));
    }

    for (auto&& [entity, field] : m_registry.view<MagneticField>().each()) {
        drawFieldRegion(field.region,
            sf::Color(0, 100, 255, 40),
            sf::Color(0, 100, 255, 180));
        drawFieldDirection(field.region, field.B, sf::Color(120, 180, 255, 230));
    }
}

void RenderSystem::drawFieldRegion(const Rect& region, sf::Color fill, sf::Color outline) {
    const float H = static_cast<float>(m_window.getSize().y);

    sf::RectangleShape rect({ region.w, region.h });
    // Flip y: top-left in screen space is the world top edge (region.y + region.h)
    rect.setPosition({ region.x, H - (region.y + region.h) });
    rect.setFillColor(fill);
    rect.setOutlineColor(outline);
    rect.setOutlineThickness(1.f);
    m_window.draw(rect);
}

void RenderSystem::drawFieldDirection(const Rect& region, const glm::vec3& vec, sf::Color color) {
    float mag = glm::length(vec);
    if (mag < kZeroEps) return;

    const float H = static_cast<float>(m_window.getSize().y);

    glm::vec2 inPlane(vec.x, vec.y);
    float inLen = glm::length(inPlane);
    float outLen = std::abs(vec.z);

    glm::vec2 dir = (inLen > kZeroEps) ? inPlane / inLen : glm::vec2(0.f);
    bool outOfScreen = vec.z > 0.f;   // +z = toward viewer (dot), -z = away (cross)

    const float margin = kGridSpacing * 0.5f;
    for (float y = region.y + margin; y < region.y + region.h; y += kGridSpacing) {
        for (float x = region.x + margin; x < region.x + region.w; x += kGridSpacing) {
            // Flip y to screen space for the grid sample point
            sf::Vector2f c{ x, H - y };
            sf::Vector2f symbolPos = c;   // defaults to center if no arrow

            // in-plane component -> arrow scaled by its share of the magnitude
            if (inLen > kZeroEps) {
                float len = std::max(kArrowMin, kArrowLength * (inLen / mag));
                // Negate y of the direction so it points correctly in screen space
                sf::Vector2f d{ dir.x, -dir.y };
                sf::Vector2f from = c - d * (len * 0.5f);
                sf::Vector2f to = c + d * (len * 0.5f);
                drawArrow(from, to, color);
                symbolPos = from;         // tuck symbol at the tail to avoid the head
            }

            // out-of-plane component -> dot/cross scaled by its share
            if (outLen > kZeroEps) {
                float r = std::max(kSymbolMin, kSymbolRadius * (outLen / mag));
                drawOutOfPlaneSymbol(symbolPos, color, outOfScreen, r);
            }
        }
    }
}

void RenderSystem::drawArrow(sf::Vector2f from, sf::Vector2f to, sf::Color color) {
    drawLine(from, to, color);

    float angle = std::atan2(to.y - from.y, to.x - from.x);
    float a1 = angle + 2.5f;   // ≈ 143°
    float a2 = angle - 2.5f;
    drawLine(to, { to.x + std::cos(a1) * kArrowHead, to.y + std::sin(a1) * kArrowHead }, color);
    drawLine(to, { to.x + std::cos(a2) * kArrowHead, to.y + std::sin(a2) * kArrowHead }, color);
}

void RenderSystem::drawOutOfPlaneSymbol(sf::Vector2f center, sf::Color color, bool outOfScreen, float r) {
    sf::CircleShape ring(r);
    ring.setOrigin({ r, r });
    ring.setPosition(center);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(color);
    ring.setOutlineThickness(1.5f);
    m_window.draw(ring);

    if (outOfScreen) {
        // ⊙ — dot (toward viewer)
        float dr = std::max(1.5f, r * 0.28f);
        sf::CircleShape dot(dr);
        dot.setOrigin({ dr, dr });
        dot.setPosition(center);
        dot.setFillColor(color);
        m_window.draw(dot);
    }
    else {
        // ⊗ — cross (away from viewer)
        float k = r * 0.7f;
        drawLine({ center.x - k, center.y - k }, { center.x + k, center.y + k }, color);
        drawLine({ center.x - k, center.y + k }, { center.x + k, center.y - k }, color);
    }
}

void RenderSystem::drawLine(sf::Vector2f a, sf::Vector2f b, sf::Color color) {
    std::array<sf::Vertex, 2> line{
        sf::Vertex{ a, color },
        sf::Vertex{ b, color }
    };
    m_window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
}