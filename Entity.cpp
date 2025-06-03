// === Entity.cpp ===
#include "Entity.h"

Entity::Entity(float x, float y, float w, float h, float health)
    : m_position(x, y), m_velocity(0, 0), m_width(w), m_height(h),
      m_maxHealth(health), m_currentHealth(health), m_alive(true) {}

void Entity::takeDamage(float damage) {
    m_currentHealth -= damage;
    if (m_currentHealth <= 0) {
        m_currentHealth = 0;
        m_alive = false;
    }
}

SDL_Rect Entity::getCollisionBox() const {
    return SDL_Rect{
        static_cast<int>(m_position.x - m_width / 2),
        static_cast<int>(m_position.y - m_height / 2),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
}

void Entity::setPosition(Vector2D newPosition) {
    m_position = newPosition;
}



