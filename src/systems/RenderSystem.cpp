#include "RenderSystem.hpp"
#include "components/Particle.hpp"
#include "components/ElectricField.hpp"
#include "components/MagneticField.hpp"
#include "components/Tags.hpp"

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

    constexpr float kParticleBaseRadius = 5.f;
    constexpr float kParticleRadiusScaleFactor = 3.5f;

    constexpr float kGradientChargeScale = 0.5f;

    uint8_t toByte(float v) {
        return static_cast<uint8_t>(std::clamp(v, 0.f, 1.f) * 255.f + 0.5f);
    }

    // Maps charge to a color gradient:
    //   negative -> red, zero -> neutral (light grey), positive -> blue.
    sf::Color chargeToColor(float charge) {
        // Squash into [-1, 1] so any magnitude works.
        float t = std::tanh(charge * kGradientChargeScale); // -1..1

        // Endpoint colors.
        const sf::Vector3f neg(1.0f, 0.25f, 0.25f); // red
        const sf::Vector3f neu(0.85f, 0.85f, 0.85f); // neutral
        const sf::Vector3f pos(0.25f, 0.45f, 1.0f); // blue

        sf::Vector3f c;
        if (t < 0.f) {
            float k = -t; // 0..1 toward red
            c = { neu.x + (neg.x - neu.x) * k,
                  neu.y + (neg.y - neu.y) * k,
                  neu.z + (neg.z - neu.z) * k };
        }
        else {
            float k = t;  // 0..1 toward blue
            c = { neu.x + (pos.x - neu.x) * k,
                  neu.y + (pos.y - neu.y) * k,
                  neu.z + (pos.z - neu.z) * k };
        }
        return sf::Color(toByte(c.x), toByte(c.y), toByte(c.z));
    }

    sf::Color brighten(sf::Color c, float amount) {
        return sf::Color(
            toByte(c.r / 255.f + amount),
            toByte(c.g / 255.f + amount),
            toByte(c.b / 255.f + amount),
            c.a);
    }
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

float calculate_particle_radius(float mass) {
    if (mass < 1.0f) mass = 1.0f;
    return kParticleBaseRadius + kParticleRadiusScaleFactor * std::log10(mass);
}

void RenderSystem::drawParticles() {
    auto view = m_registry.view<Particle>();

    const float H = static_cast<float>(m_window.getSize().y);

    // Instantiate a reusable circle shape. 
    // We don't give it a size yet since it changes per particle.
    sf::CircleShape circle;

    for (auto&& [entity, particle] : view.each()) {
        const sf::Vector2f pos{ particle.position.x, H - particle.position.y };
        const auto hl = highlightOf(entity);

        // 1. Calculate the dynamic radius based on this particle's mass
        float current_radius = calculate_particle_radius(particle.mass);

        // 2. Update the circle shape properties for this specific particle
        circle.setRadius(current_radius);
        circle.setOrigin({ current_radius, current_radius }); // Centers the origin correctly

        sf::Color fill = chargeToColor(particle.charge);

        if (hl.hovered && !hl.selected) {
            fill = brighten(fill, 0.20f);
        }

        circle.setPosition(pos);
        circle.setFillColor(fill);

        if (hl.hovered && !hl.selected) {
            circle.setOutlineThickness(1.5f);
            circle.setOutlineColor(sf::Color(255, 255, 255, 180));
        }
        else {
            circle.setOutlineThickness(0.f);
            circle.setOutlineColor(sf::Color::Transparent);
        }

        m_window.draw(circle);

        if (hl.selected) {
            // 3. Pass the dynamic radius here too so the selection ring matches!
            drawSelectionOutline(pos, current_radius);
        }
    }
}

void RenderSystem::drawSelectionOutline(sf::Vector2f center, float baseRadius) {
    // Outer bright ring.
    const float outerR = baseRadius + 5.f;
    sf::CircleShape ring(outerR);
    ring.setOrigin({ outerR, outerR });
    ring.setPosition(center);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(255, 215, 0, 230)); // gold highlight
    ring.setOutlineThickness(2.5f);
    m_window.draw(ring);

    // Soft halo for extra emphasis.
    const float haloR = baseRadius + 9.f;
    sf::CircleShape halo(haloR);
    halo.setOrigin({ haloR, haloR });
    halo.setPosition(center);
    halo.setFillColor(sf::Color::Transparent);
    halo.setOutlineColor(sf::Color(255, 215, 0, 60));
    halo.setOutlineThickness(2.f);
    m_window.draw(halo);
}

void RenderSystem::drawSelectionOutline(const Rect& region) {
    const float H = static_cast<float>(m_window.getSize().y);
    const float pad = 3.f;

    // Bright frame.
    sf::RectangleShape frame({ region.w + pad * 2.f, region.h + pad * 2.f });
    frame.setPosition({ region.x - pad, H - (region.y + region.h) - pad });
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineColor(sf::Color(255, 215, 0, 230)); // gold highlight
    frame.setOutlineThickness(2.5f);
    m_window.draw(frame);

    // Soft halo.
    const float halo = pad + 4.f;
    sf::RectangleShape glow({ region.w + halo * 2.f, region.h + halo * 2.f });
    glow.setPosition({ region.x - halo, H - (region.y + region.h) - halo });
    glow.setFillColor(sf::Color::Transparent);
    glow.setOutlineColor(sf::Color(255, 215, 0, 60));
    glow.setOutlineThickness(2.f);
    m_window.draw(glow);
}

void RenderSystem::drawField(entt::entity entity, const Rect& region, const glm::vec3& vec,
    sf::Color fill, sf::Color outline, sf::Color arrow) {
    const auto hl = highlightOf(entity);

    // Hovered: brighten the region so it reads as "live" without changing layout.
    if (hl.hovered && !hl.selected) {
        fill = brighten(fill, 0.15f);
        outline = brighten(outline, 0.30f);
    }

    drawFieldRegion(region, fill, outline);
    drawFieldDirection(region, vec, arrow);

    if (hl.selected) {
        drawSelectionOutline(region);
    }
}

void RenderSystem::drawFields() {
    for (auto&& [entity, field] : m_registry.view<ElectricField>().each()) {
        drawField(entity, field.region, field.E,
            sf::Color(255, 255, 0, 40),
            sf::Color(255, 255, 0, 180),
            sf::Color(255, 255, 0, 220));
    }

    for (auto&& [entity, field] : m_registry.view<MagneticField>().each()) {
        drawField(entity, field.region, field.B,
            sf::Color(0, 100, 255, 40),
            sf::Color(0, 100, 255, 180),
            sf::Color(120, 180, 255, 230));
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

Highlight RenderSystem::highlightOf(entt::entity e) const {
    return { m_registry.all_of<Hovered>(e),
             m_registry.all_of<Selected>(e) };
}