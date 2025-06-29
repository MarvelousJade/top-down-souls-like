#ifndef BOSS_H
#define BOSS_H

#include "Entity.h"
#include "Vector2D.h"
#include <cmath>
#include <algorithm>

extern float bossAttackRange;

// Boss visual/animation states
enum class BossAnimState {
    IDLE,
    MOVING,
    ATTACKING,
    RECOVERING,
    DAMAGED,
    DYING
};

// Attack animation types that map to Sif's AttackType
enum class BossAttackAnim {
    HORIZONTAL_SWING,
    SPIN_ATTACK,
    OVERHEAD_SWING,
    UPPERCUT,
    GROUND_SLAM,
    DASH_ATTACK,
    PROJECTILE,
    BACKSTEP_SLASH
};

class Boss : public Entity {
private:
    // Animation state
    BossAnimState m_animState;
    BossAttackAnim m_currentAttackAnim;
    float m_animTimer;
    float m_animDuration;
    float m_windupTimer;
    float m_windupDuration;
    
    // Visual properties
    Vector2D m_facingDirection;
    float m_swordAngle;
    float m_swordLength;
    Vector2D m_swordBase;
    Vector2D m_swordTipPosition;
    bool m_swordOnRightSide;
    
    // Combat properties
    float m_baseAttackDamage;
    float m_currentAttackDamage;
    float m_attackRange;
    bool m_hasDealtDamage;
    
    // Movement properties
    float m_moveSpeed;
    float m_currentMoveSpeed;
    Vector2D m_targetMovePosition;
    
    // Helper methods
    void updateSwordPosition();
    void updateAnimation(float deltaTime);
    
public:
    Boss(float x, float y);
    
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    
    // AI Interface - These are called by Sif AI
    void setFacingDirection(const Vector2D& direction);
    void startAttackAnimation(BossAttackAnim attackType);
    void startMoving(const Vector2D& targetPos, float speed = 1.0f);
    void stopMoving();
    void performStep(const Vector2D& direction, float distance);
    void cancelAttack();
    void forceIdle();

    // State queries for AI
    bool isAttacking() const;
    bool isRecovering() const { return m_animState == BossAnimState::RECOVERING; }
    bool isMoving() const { return m_animState == BossAnimState::MOVING; };
    bool canAct() const { return m_animState == BossAnimState::IDLE || m_animState == BossAnimState::MOVING; }
    bool isWindingUp() const;
    float getAttackRange() const { return m_attackRange; }
    float getAttackDamage() const { return m_currentAttackDamage; }
    float getAnimationProgress() const;
    Vector2D getSwordBase() const { return m_swordBase; }

    // Combat
    Circle getAttackCircle() const;
    SDL_Rect getSwordHitbox() const;
    bool hasDealtDamage() const { return m_hasDealtDamage; }
    void setDamageDealt() { m_hasDealtDamage = true; }
    void takeDamage(float damage) override;

    // Getter methods for rendering
    BossAnimState getAnimState() const { return m_animState; }
    BossAttackAnim getCurrentAttackAnim() const { return m_currentAttackAnim; }
};

#endif
