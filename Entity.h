// === Entity.h ===
#ifndef ENTITY_H
#define ENTITY_H

#include "Vector2D.h"
#include <SDL2/SDL.h>

struct Circle {
    float x, y, r;
    Circle(float x, float y, float r = 0) 
       : x(x), y(y), r(r) {}
};

class Entity {
protected:
    Vector2D m_position;
    Vector2D m_velocity;
    float m_width;
    float m_height;
    float m_maxHealth;
    float m_currentHealth;
    bool m_alive;
   
public:
    Entity(float x, float y, float w, float h, float health);
    virtual ~Entity() = default;
    
    virtual void update(float deltaTime) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
    
    void takeDamage(float damage);
    bool isAlive() const { return m_alive; }
    
    Vector2D getPosition() const { return m_position; }
    SDL_Rect getCollisionBox() const;
    float getHealthPercentage() const { return m_currentHealth / m_maxHealth; }
    
    // Getters for dimensions
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    
    void setPosition(Vector2D newPosition);
};

#endif
