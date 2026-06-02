// ui/InteractionHandler.cpp
#include "InteractionHandler.hpp"
#include "ui/Inspector.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

namespace {
    constexpr float kParticlePickRadiusSq = 36.f; // 6px radius squared

    bool hitParticle(const Particle& p, sf::Vector2f worldPos) {
        float dx = p.position.x - worldPos.x;
        float dy = p.position.y - worldPos.y;
        return dx * dx + dy * dy < kParticlePickRadiusSq;
    }

    InteractionHandler::Handle pickHandle(const Rect& r, sf::Vector2f p, float t) {
        using H = InteractionHandler::Handle;

        bool withinX = p.x >= r.x - t && p.x <= r.x + r.w + t;
        bool withinY = p.y >= r.y - t && p.y <= r.y + r.h + t;
        if (!withinX || !withinY) return H::None;

        bool left = std::abs(p.x - r.x) <= t;
        bool right = std::abs(p.x - (r.x + r.w)) <= t;
        bool top = std::abs(p.y - r.y) <= t;
        bool bottom = std::abs(p.y - (r.y + r.h)) <= t;

        if (top && left)  return H::TopLeft;
        if (top && right) return H::TopRight;
        if (bottom && left)  return H::BottomLeft;
        if (bottom && right) return H::BottomRight;
        if (left)   return H::Left;
        if (right)  return H::Right;
        if (top)    return H::Top;
        if (bottom) return H::Bottom;
        return H::None;
    }

    void applyResize(Rect& r, InteractionHandler::Handle h, sf::Vector2f p, float minSize) {
        using H = InteractionHandler::Handle;

        float left = r.x, top = r.y, right = r.x + r.w, bottom = r.y + r.h;

        switch (h) {
        case H::Left:        left = p.x; break;
        case H::Right:       right = p.x; break;
        case H::Top:         top = p.y; break;
        case H::Bottom:      bottom = p.y; break;
        case H::TopLeft:     left = p.x; top = p.y; break;
        case H::TopRight:    right = p.x; top = p.y; break;
        case H::BottomLeft:  left = p.x; bottom = p.y; break;
        case H::BottomRight: right = p.x; bottom = p.y; break;
        default: break;
        }

        // normalize so dragging an edge past the opposite one just flips it
        r.x = std::min(left, right);
        r.y = std::min(top, bottom);
        r.w = std::max(std::abs(right - left), minSize);
        r.h = std::max(std::abs(bottom - top), minSize);
    }
}

InteractionHandler::InteractionHandler(entt::registry& registry, Inspector& inspector)
    : m_registry(registry)
    , m_inspector(inspector)
{
}

std::array<sf::Vector2f, 8> InteractionHandler::handlePositions(const Rect& r) {
    float l = r.x, t = r.y, rt = r.x + r.w, b = r.y + r.h;
    float cx = r.x + r.w * 0.5f, cy = r.y + r.h * 0.5f;
    // order matches the Handle enum (TopLeft..BottomRight)
    return { {
        { l,  t  }, { cx, t  }, { rt, t  },
        { l,  cy },             { rt, cy },
        { l,  b  }, { cx, b  }, { rt, b  }
    } };
}

void InteractionHandler::handleEvent(const sf::Event& event, sf::RenderWindow& window, SimulationMode mode) {
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f worldPos = toWorldPos(window);

        if (mode == SimulationMode::Editing) {
            trySelect(worldPos);
            beginDrag(worldPos);
        }
    }

    if (const auto* mouse = event.getIf<sf::Event::MouseMoved>()) {
        if (m_isDragging)
            updateDrag(toWorldPos(window));
    }

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>()) {
        endDrag();
    }
}

void InteractionHandler::trySelect(sf::Vector2f worldPos) {
    for (auto&& [entity, particle] : m_registry.view<Particle>().each()) {
        if (hitParticle(particle, worldPos)) {
            m_inspector.select(entity);
            return;
        }
    }
    for (auto&& [entity, field] : m_registry.view<ElectricField>().each()) {
        if (field.region.contains(worldPos.x, worldPos.y)) {
            m_inspector.select(entity);
            return;
        }
    }
    for (auto&& [entity, field] : m_registry.view<MagneticField>().each()) {
        if (field.region.contains(worldPos.x, worldPos.y)) {
            m_inspector.select(entity);
            return;
        }
    }
    m_inspector.deselect();
}

Rect* InteractionHandler::getRegion(entt::entity e) {
    if (auto* ef = m_registry.try_get<ElectricField>(e)) return &ef->region;
    if (auto* mf = m_registry.try_get<MagneticField>(e)) return &mf->region;
    return nullptr;
}

void InteractionHandler::beginDrag(sf::Vector2f worldPos) {
    m_resizeHandle = Handle::None;

    // particles first
    for (auto&& [entity, particle] : m_registry.view<Particle>().each()) {
        if (hitParticle(particle, worldPos)) {
            m_dragging = entity;
            m_isDragging = true;
            m_dragOffset = { particle.position.x - worldPos.x,
                             particle.position.y - worldPos.y };
            return;
        }
    }

    // fields: edge/corner = resize, interior = move
    auto tryField = [&](entt::entity entity, Rect& region) -> bool {
        Handle h = pickHandle(region, worldPos, kHandleSize);
        if (h != Handle::None) {
            m_dragging = entity;
            m_isDragging = true;
            m_resizeHandle = h;
            return true;
        }
        if (region.contains(worldPos.x, worldPos.y)) {
            m_dragging = entity;
            m_isDragging = true;
            m_dragOffset = { region.x - worldPos.x, region.y - worldPos.y };
            return true;
        }
        return false;
        };

    for (auto&& [entity, field] : m_registry.view<ElectricField>().each())
        if (tryField(entity, field.region)) return;

    for (auto&& [entity, field] : m_registry.view<MagneticField>().each())
        if (tryField(entity, field.region)) return;
}

void InteractionHandler::updateDrag(sf::Vector2f worldPos) {
    if (m_dragging == entt::null) return;
    if (!m_registry.valid(m_dragging)) { endDrag(); return; }

    if (auto* particle = m_registry.try_get<Particle>(m_dragging)) {
        particle->position.x = worldPos.x + m_dragOffset.x;
        particle->position.y = worldPos.y + m_dragOffset.y;
        return;
    }

    if (Rect* region = getRegion(m_dragging)) {
        if (m_resizeHandle != Handle::None) {
            applyResize(*region, m_resizeHandle, worldPos, kMinFieldSize);
        }
        else {
            region->x = worldPos.x + m_dragOffset.x;
            region->y = worldPos.y + m_dragOffset.y;
        }
    }
}

void InteractionHandler::endDrag() {
    m_dragging = entt::null;
    m_isDragging = false;
    m_resizeHandle = Handle::None;
}

void InteractionHandler::spawnParticle(sf::Vector2f worldPos) {
    auto entity = m_registry.create();
    m_registry.emplace<Particle>(entity,
        glm::vec3(worldPos.x, worldPos.y, 0.f),
        glm::vec3(0.f), 1.f, 1.f);
    m_inspector.select(entity);
}

void InteractionHandler::spawnElectricField(sf::Vector2f worldPos) {
    auto entity = m_registry.create();
    m_registry.emplace<ElectricField>(entity,
        glm::vec3(1.f, 0.f, 0.f),
        Rect{ worldPos.x, worldPos.y, 100.f, 100.f });
    m_inspector.select(entity);
}

void InteractionHandler::spawnMagneticField(sf::Vector2f worldPos) {
    auto entity = m_registry.create();
    m_registry.emplace<MagneticField>(entity,
        glm::vec3(0.f, 0.f, 1.f),
        Rect{ worldPos.x, worldPos.y, 100.f, 100.f });
    m_inspector.select(entity);
}

sf::Vector2f InteractionHandler::toWorldPos(sf::RenderWindow& window) {
    sf::Vector2f pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
	pos.y = static_cast<float>(window.getSize().y) - pos.y; // flip y

    return pos;
}