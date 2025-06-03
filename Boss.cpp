#include "Boss.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Boss::Boss(float x, float y)
    : Entity(x, y, 80, 60, 300),
      m_animState(BossAnimState::IDLE),
      m_currentAttackAnim(BossAttackAnim::HORIZONTAL_SWING),
      m_animTimer(0.0f),
      m_animDuration(0.0f),
      m_facingDirection(0, 1),
      m_swordAngle(0.0f),
      m_swordLength(100.0f),
      m_swordOnRightSide(true),
      m_baseAttackDamage(25.0f),
      m_currentAttackDamage(25.0f),
      m_attackRange(120.0f),
      m_hasDealtDamage(false),
      m_moveSpeed(310.0f) {
    updateSwordPosition();
}

void Boss::update(float deltaTime) {
    // Update animation
    updateAnimation(deltaTime);
    
    // Handle movement
    if (m_animState == BossAnimState::MOVING) {
        Vector2D toTarget = m_targetMovePosition - m_position;
        float distance = toTarget.length();
        
        if (distance > 5.0f) {
            Vector2D movement = toTarget.normalized() * m_moveSpeed * deltaTime;
            m_position = m_position + movement;
            
            // Keep in bounds
            m_position.x = std::max(80.0f, std::min(720.0f, m_position.x));
            m_position.y = std::max(80.0f, std::min(520.0f, m_position.y));
        } else {
            // Reached target
            m_animState = BossAnimState::IDLE;
        }
    }
    
    updateSwordPosition();
}

void Boss::updateAnimation(float deltaTime) {
    if (m_animTimer > 0) {
        m_animTimer -= deltaTime;
        
        // Handle different attack animations
        if (m_animState == BossAnimState::ATTACKING) {
            switch (m_currentAttackAnim) {
                case BossAttackAnim::HORIZONTAL_SWING:
                    if (m_swordOnRightSide) {
                        m_swordAngle = (M_PI * (1.0f - m_animTimer / m_animDuration));
                    } else {
                        m_swordAngle = M_PI - (M_PI * (1.0f - m_animTimer / m_animDuration));
                    }
                    break;
                    
                case BossAttackAnim::SPIN_ATTACK:
                    m_swordAngle += (2 * M_PI * deltaTime / m_animDuration);
                    break;
                    
                case BossAttackAnim::OVERHEAD_SWING:
                    if (m_animTimer > m_animDuration * 0.5f) {
                        m_swordAngle = -M_PI * 0.8f;
                    } else {
                        m_swordAngle = M_PI * 0.5f;
                    }
                    break;
                    
                case BossAttackAnim::UPPERCUT:
                    m_swordAngle = -M_PI * 0.5f + (M_PI * 0.5f * (1.0f - m_animTimer / m_animDuration));
                    break;
                    
                case BossAttackAnim::GROUND_SLAM:
                    if (m_animTimer > m_animDuration * 0.6f) {
                        m_swordAngle = -M_PI * 0.7f;
                    } else {
                        m_swordAngle = M_PI * 0.5f;
                    }
                    break;
                    
                case BossAttackAnim::DASH_ATTACK:
                    m_swordAngle = m_swordOnRightSide ? -M_PI * 0.2f : M_PI * 1.2f;
                    break;
                    
                case BossAttackAnim::BACKSTEP_SLASH:
                    m_swordAngle = m_swordOnRightSide ? M_PI * 0.3f : M_PI * 0.7f;
                    break;
                    
                case BossAttackAnim::PROJECTILE:
                    m_swordAngle = -M_PI * 0.4f + sin(m_animTimer * 10) * 0.1f;
                    break;
            }
        }
        
        // Transition to recovery or idle
        if (m_animTimer <= 0) {
            if (m_animState == BossAnimState::ATTACKING) {
                m_animState = BossAnimState::RECOVERING;
                m_animTimer = 0.3f;
                m_animDuration = 0.3f;
                
                // Some attacks switch sword side
                if (m_currentAttackAnim == BossAttackAnim::HORIZONTAL_SWING) {
                    m_swordOnRightSide = !m_swordOnRightSide;
                }
            } else if (m_animState == BossAnimState::RECOVERING) {
                m_animState = BossAnimState::IDLE;
                m_swordAngle = m_swordOnRightSide ? 0.0f : M_PI;
            } else if (m_animState == BossAnimState::DAMAGED) {
                m_animState = BossAnimState::IDLE;
            }
        }
    }
    
    // Idle animation
    if (m_animState == BossAnimState::IDLE) {
        m_swordAngle = m_swordOnRightSide ? 0.0f : M_PI;
        m_swordAngle += sin(SDL_GetTicks() * 0.001f * 1.5f) * 0.05f;
    }
}

void Boss::render(SDL_Renderer* renderer) {
    // Draw wolf body
    SDL_Color bodyColor;
    bool isInjured = m_currentHealth < m_maxHealth * 0.3f;
    
    switch (m_animState) {
        case BossAnimState::ATTACKING:
            bodyColor = isInjured ? SDL_Color{80, 80, 80, 255} : SDL_Color{100, 100, 100, 255};
            break;
        case BossAnimState::DAMAGED:
            bodyColor = SDL_Color{150, 50, 50, 255};
            break;
        case BossAnimState::MOVING:
            bodyColor = isInjured ? SDL_Color{110, 110, 110, 255} : SDL_Color{130, 130, 130, 255};
            break;
        default:
            bodyColor = isInjured ? SDL_Color{100, 100, 100, 255} : SDL_Color{120, 120, 120, 255};
            break;
    }
    
    SDL_SetRenderDrawColor(renderer, bodyColor.r, bodyColor.g, bodyColor.b, bodyColor.a);
    SDL_Rect rect = getCollisionBox();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw injuries when health is low
    if (isInjured) {
        SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255);
        SDL_Rect injuryRect = {rect.x + 10, rect.y + 20, 10, 5};
        SDL_RenderFillRect(renderer, &injuryRect);
        injuryRect.x += 20;
        injuryRect.y -= 10;
        SDL_RenderFillRect(renderer, &injuryRect);
    }
    
    // Draw sword
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    
    Vector2D swordBase = m_position + m_facingDirection * 30;
    Vector2D swordEnd = swordBase + Vector2D(cos(m_swordAngle), sin(m_swordAngle)) * m_swordLength;
    
    // Draw sword as thick line
    for (int i = -2; i <= 2; i++) {
        SDL_RenderDrawLine(renderer, 
            swordBase.x + i, swordBase.y,
            swordEnd.x + i, swordEnd.y);
        SDL_RenderDrawLine(renderer, 
            swordBase.x, swordBase.y + i,
            swordEnd.x, swordEnd.y + i);
    }
    
    // Draw sword hilt
    SDL_SetRenderDrawColor(renderer, 150, 100, 50, 255);
    SDL_Rect hiltRect = {
        (int)swordBase.x - 5,
        (int)swordBase.y - 5,
        10, 10
    };
    SDL_RenderFillRect(renderer, &hiltRect);
    
    // Draw eyes
    SDL_SetRenderDrawColor(renderer, isInjured ? 100 : 255, 50, 50, 255);
    Vector2D eyeOffset(-10, -10);
    Vector2D eyePos = m_position + eyeOffset;
    SDL_Rect eyeRect = {(int)eyePos.x - 2, (int)eyePos.y - 2, 4, 4};
    SDL_RenderFillRect(renderer, &eyeRect);
    eyeOffset.x = 10;
    eyePos = m_position + eyeOffset;
    eyeRect = {(int)eyePos.x - 2, (int)eyePos.y - 2, 4, 4};
    SDL_RenderFillRect(renderer, &eyeRect);
}

void Boss::setFacingDirection(const Vector2D& direction) {
    m_facingDirection = direction.normalized();
    updateSwordPosition();
}

void Boss::startAttackAnimation(BossAttackAnim attackType) {
    if (!canAct()) return;
    
    m_animState = BossAnimState::ATTACKING;
    m_currentAttackAnim = attackType;
    m_hasDealtDamage = false;
    
    // Set animation durations and damage
    switch (attackType) {
        case BossAttackAnim::HORIZONTAL_SWING:
            m_animDuration = 0.5f;
            m_currentAttackDamage = m_baseAttackDamage;
            break;
        case BossAttackAnim::SPIN_ATTACK:
            m_animDuration = 0.8f;
            m_currentAttackDamage = m_baseAttackDamage * 1.2f;
            break;
        case BossAttackAnim::OVERHEAD_SWING:
            m_animDuration = 0.7f;
            m_currentAttackDamage = m_baseAttackDamage * 1.5f;
            break;
        case BossAttackAnim::UPPERCUT:
            m_animDuration = 0.6f;
            m_currentAttackDamage = m_baseAttackDamage * 1.3f;
            break;
        case BossAttackAnim::GROUND_SLAM:
            m_animDuration = 0.8f;
            m_currentAttackDamage = m_baseAttackDamage * 1.8f;
            break;
        case BossAttackAnim::DASH_ATTACK:
            m_animDuration = 0.4f;
            m_currentAttackDamage = m_baseAttackDamage;
            break;
        case BossAttackAnim::BACKSTEP_SLASH:
            m_animDuration = 0.5f;
            m_currentAttackDamage = m_baseAttackDamage * 1.1f;
            break;
        case BossAttackAnim::PROJECTILE:
            m_animDuration = 1.0f;
            m_currentAttackDamage = m_baseAttackDamage * 0.8f;
            break;
    }
    
    m_animTimer = m_animDuration;
}

void Boss::startMoving(const Vector2D& targetPos, float speed) {
    if (!canAct()) return;
    
    m_animState = BossAnimState::MOVING;
    m_targetMovePosition = targetPos;
    m_moveSpeed = 310.0f * speed;
    m_facingDirection = (targetPos - m_position).normalized();
}

void Boss::stopMoving() {
    if (m_animState == BossAnimState::MOVING) {
        m_animState = BossAnimState::IDLE;
    }
}

void Boss::performStep(const Vector2D& direction, float distance) {
    if (!canAct()) return;
    
    Vector2D newPos = m_position + direction.normalized() * distance;
    
    // Keep in bounds
    newPos.x = std::max(80.0f, std::min(720.0f, newPos.x));
    newPos.y = std::max(80.0f, std::min(520.0f, newPos.y));
    
    m_position = newPos;
}

void Boss::takeDamage(float damage) {
    Entity::takeDamage(damage);
    
    if (m_animState != BossAnimState::ATTACKING) {
        m_animState = BossAnimState::DAMAGED;
        m_animTimer = 0.2f;
        m_animDuration = 0.2f;
    }
}

void Boss::updateSwordPosition() {
    Vector2D swordBase = m_position + m_facingDirection * 30;
    m_swordTipPosition = swordBase + Vector2D(cos(m_swordAngle), sin(m_swordAngle)) * m_swordLength;
}

Circle Boss::getAttackCircle() const {
    if (m_currentAttackAnim == BossAttackAnim::SPIN_ATTACK) {
        return Circle(m_position.x, m_position.y, m_swordLength + 40);
    }
    return Circle(m_swordTipPosition.x, m_swordTipPosition.y, 30);
}

SDL_Rect Boss::getSwordHitbox() const {
    int x = std::min(m_position.x, m_swordTipPosition.x) - 10;
    int y = std::min(m_position.y, m_swordTipPosition.y) - 10;
    int w = std::abs(m_swordTipPosition.x - m_position.x) + 20;
    int h = std::abs(m_swordTipPosition.y - m_position.y) + 20;
    return SDL_Rect{x, y, w, h};
}
