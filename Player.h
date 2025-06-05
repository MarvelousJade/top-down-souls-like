#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "GameUnits.h"

enum class PlayerState {
    IDLE,
    MOVING,
    ATTACKING,
    DODGING,
    STUNNED
};

class Player : public Entity {
private:
    PlayerState m_state;
    float m_speed;
    float m_maxStamina;
    float m_currentStamina;
    float m_staminaRegenRate;
    float m_staminaRegenDelay;  // 1.5 second delay before stamina starts regenerating
    float m_timeSinceStaminaUse;

    // Combat
    float m_attackCooldown;
    float m_attackDamage;
    float m_attackRange;
    bool m_hasDealtDamage;  // Flag to ensure damage is only dealt once per attack

    // Sword properties
    float m_swordAngle;
    float m_swordLength;
    Vector2D m_swordTipPosition;
    Vector2D m_facingDirection;
    // Vector2D m_targetPosition;  // Boss position for sword tracking

    // Dodge
    float m_dodgeCooldown;
    float m_dodgeDuration;
    float m_dodgeSpeed;
    Vector2D m_dodgeDirection;
    
    // State timers
    float m_stateTimer;
    float m_animationTimer;
    
    // Window boundaries
    float m_windowWidth;
    float m_windowHeight;
    void updateSwordPosition();
    
public:
    Player(float x, float y);
    
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    
    void move(const Vector2D& direction);
    void attack();
    void dodge(const Vector2D& direction);
    void setWindowBounds(float width, float height) { 
        m_windowWidth = GameUnits::toMeters(width); 
        m_windowHeight = GameUnits::toMeters(height); 
    }
    
    bool canAttack() const;
    bool canDodge() const;
    bool isInvulnerable() const;
    bool isAttacking() const { return m_state == PlayerState::ATTACKING; }
    
    float getAttackDamage() const { return m_attackDamage; }
    float getAttackRange() const { return m_attackRange; }
    float getStaminaPercentage() const { return m_currentStamina / m_maxStamina; }
    PlayerState getState() const { return m_state; }
    bool hasDealtDamage() const { return m_hasDealtDamage; }
    void setDamageDealt() { m_hasDealtDamage = true; }

    // Get sword hitbox for collision detection
    SDL_Rect getSwordHitbox() const;
    Circle getAttackCircle() const { return Circle(m_position.x, m_position.y, m_attackRange); }
};

#endif
