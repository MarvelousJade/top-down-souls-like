#include "Entity.h"
#include "GameUnits.h"

Entity::Entity(float x, float y, float w, float h, float health)
    : m_position(GameUnits::toMeters(x), GameUnits::toMeters(y)),
      m_width(GameUnits::toMeters(w)), 
      m_height(GameUnits::toMeters(h)),
      m_maxHealth(health), m_currentHealth(health), m_alive(true) {}

void Entity::takeDamage(float damage) {
    m_currentHealth -= damage;
    if (m_currentHealth <= 0) {
        m_currentHealth = 0;
        m_alive = false;
    }
}

SDL_Rect Entity::getCollisionBox() const {
    Vector2D pixelPos = GameUnits::toPixels(m_position);
    return SDL_Rect{
        (int)(pixelPos.x - GameUnits::toPixels(m_width) / 2),
        (int)(pixelPos.y - GameUnits::toPixels(m_height) / 2),
        (int)GameUnits::toPixels(m_width),
        (int)GameUnits::toPixels(m_height)
    };
}




