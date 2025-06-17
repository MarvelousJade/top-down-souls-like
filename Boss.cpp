#include "Boss.h"
#include "GameUnits.h"
#include "Sif.h"
#include "Vector2D.h"
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float bossAttackRange = 6.0f;

Boss::Boss(float x, float y)
    : Entity(x, y, 60, 120, 300),
      m_animState(BossAnimState::IDLE),
      m_currentAttackAnim(BossAttackAnim::HORIZONTAL_SWING),
      m_animTimer(0.0f),
      m_animDuration(0.0f),
      m_windupTimer(0.0f),
      m_windupDuration(0.0f),
      m_facingDirection(0, 1),
      m_swordAngle(0.0f),
      m_swordLength(6.0f),
      m_swordOnRightSide(true),
      m_baseAttackDamage(25.0f),
      m_currentAttackDamage(25.0f),
      m_attackRange(bossAttackRange),
      m_hasDealtDamage(false),
      m_moveSpeed(8.0f),
    m_currentMoveSpeed(8.0f) {
    updateSwordPosition();
}

void Boss::update(float deltaTime) {
    // Update animation
    updateAnimation(deltaTime);
   
    // Handle movement
    if (m_animState == BossAnimState::MOVING) {
        Vector2D toTarget = m_targetMovePosition - m_position;
        float distance = toTarget.length();


        if (distance > 0.5f) {
            // Smooth acceleration/deceleration
            float speedModifier = 1.0f;
            if (distance < 2.0f) {
                // Slow down when approaching target
                speedModifier = distance / 2.0f;
            } else if (distance > 10.0f) {
                // Speed up for long distances
                speedModifier = 1.5f;
            }

            Vector2D movement = toTarget.normalized() * m_moveSpeed * deltaTime;

            // Prevent overshooting
            if (movement.length() > distance) {
                m_position = m_targetMovePosition;
                m_animState = BossAnimState::IDLE;
            } else {
                m_position = m_position + movement;
            }

            // Update facing direction while moving
            m_facingDirection = toTarget.normalized();
            
            // Keep in bounds
            float minX = GameUnits::toMeters(60.0f);  // 2.67 meters
            float maxX = GameUnits::toMeters(740.0f); // 24 meters
            float minY = GameUnits::toMeters(120.0f);  // 2.67 meters
            float maxY = GameUnits::toMeters(480.0f); // 17.33 meters
            
            m_position.x = (std::max(minX, std::min(maxX, m_position.x)));
            m_position.y = (std::max(minY, std::min(maxY, m_position.y)));
        } else {
            // Reached target
            m_animState = BossAnimState::IDLE;
        }
    }
    updateSwordPosition();
}

void Boss::updateAnimation(float deltaTime) {
    // Handle wind-up phase
    if (m_windupTimer > 0) {
        m_windupTimer -= deltaTime;
        
        // Wind-up animation (pull sword back)
        float windupProgress = 1.0f - (m_windupTimer / m_windupDuration);
        
        switch (m_currentAttackAnim) {
            case BossAttackAnim::HORIZONTAL_SWING:
                m_swordAngle = m_swordOnRightSide ? -M_PI * 0.3f * windupProgress : M_PI + M_PI * 0.3f * windupProgress;
                break;
            case BossAttackAnim::OVERHEAD_SWING:
            case BossAttackAnim::GROUND_SLAM:
                m_swordAngle = -M_PI * 0.8f * windupProgress;
                break;
            case BossAttackAnim::SPIN_ATTACK:
                // Slight pullback before spin
                m_swordAngle = -M_PI * 0.2f * windupProgress;
                break;
            case BossAttackAnim::UPPERCUT:
                m_swordAngle = M_PI * 0.4f * windupProgress;
                break;
            default:
                m_swordAngle = -M_PI * 0.2f * windupProgress;
                break;
        }
        
        return; // Don't process attack animation while winding up
    }

    // Regular animation processing
    if (m_animTimer > 0) {
        m_animTimer -= deltaTime;
        
        // Handle different attack animations
        if (m_animState == BossAnimState::ATTACKING) {
            float attackProgress = 1.0f - (m_animTimer / m_animDuration);

            switch (m_currentAttackAnim) {
                case BossAttackAnim::HORIZONTAL_SWING:
                    if (m_swordOnRightSide) {
                        m_swordAngle = -M_PI * 0.3f + (M_PI * 1.3f * attackProgress);
                    } else {
                        m_swordAngle = M_PI * 1.3f - (M_PI * 1.3f * attackProgress);
                    }
                    break;
                    
                case BossAttackAnim::SPIN_ATTACK:
                    m_swordAngle += (2 * M_PI * deltaTime / m_animDuration) * 2;
                    break;
                    
                case BossAttackAnim::OVERHEAD_SWING:
                    m_swordAngle = -M_PI * 0.8f + (M_PI * 1.3f * attackProgress);
                    break;                    

                case BossAttackAnim::UPPERCUT:
                    m_swordAngle = M_PI * 0.4f - (M_PI * 0.9f * attackProgress);
                    break;
                    
                case BossAttackAnim::GROUND_SLAM:
                    if (attackProgress < 0.3f) {
                        // Stay raised
                        m_swordAngle = -M_PI * 0.8f;
                    } else {
                        // Slam down
                        float slamProgress = (attackProgress - 0.3f) / 0.7f;
                        m_swordAngle = -M_PI * 0.8f + (M_PI * 1.3f * slamProgress);
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

     // Add wind-up visual indicator
    if (m_windupTimer > 0) {
        // Flash white during wind-up
        float flashIntensity = sin(m_windupTimer * 20) * 0.5f + 0.5f;
        bodyColor.r = std::min(255, (int)(bodyColor.r + flashIntensity * 50));
        bodyColor.g = std::min(255, (int)(bodyColor.g + flashIntensity * 50));
        bodyColor.b = std::min(255, (int)(bodyColor.b + flashIntensity * 50));
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
    SDL_Color swordColor = {200, 200, 200, 255};

    // Make sword glow during wind-up
    if (m_windupTimer > 0) {
        float glowIntensity = 1.0f - (m_windupTimer / m_windupDuration);
        swordColor.r = std::min(255, (int)(200 + glowIntensity * 55));
        swordColor.g = std::min(255, (int)(200 + glowIntensity * 30));
        swordColor.b = std::min(255, (int)(200 + glowIntensity * 30));
    }

    SDL_SetRenderDrawColor(renderer, swordColor.r, swordColor.g, swordColor.b, swordColor.a);
    
    Vector2D m_swordBase = m_position + m_facingDirection * GameUnits::toMeters(30);
    Vector2D swordEnd = m_swordBase + Vector2D(cos(m_swordAngle), sin(m_swordAngle)) * m_swordLength;
    
    Vector2D pixelBase = GameUnits::toPixels(m_swordBase);
    Vector2D pixelEnd = GameUnits::toPixels(swordEnd);

    // Draw sword as thick line
    for (int i = -2; i <= 2; i++) {
        SDL_RenderDrawLine(renderer, 
            pixelBase.x + i, pixelBase.y,
            pixelEnd.x + i, pixelEnd.y);
        SDL_RenderDrawLine(renderer, 
            pixelBase.x, pixelBase.y + i,
            pixelEnd.x, pixelEnd.y + i);
    }
    
    // Draw sword hilt
    SDL_SetRenderDrawColor(renderer, 150, 100, 50, 255);
    SDL_Rect hiltRect = {
        (int)pixelBase.x - 5,
        (int)pixelBase.y - 5,
        10, 10
    };
    SDL_RenderFillRect(renderer, &hiltRect);
    
    // Draw eyes
    Vector2D pixelPos = GameUnits::toPixels(m_position);
    SDL_SetRenderDrawColor(renderer, isInjured ? 100 : 255, 50, 50, 255);

    // Make eyes glow during wind-up
    if (m_windupTimer > 0) {
        SDL_SetRenderDrawColor(renderer, 255, 200, 50, 255);
    }

    Vector2D eyeOffset(-10, -10);
    Vector2D eyePos = pixelPos + eyeOffset;
    SDL_Rect eyeRect = {(int)eyePos.x - 2, (int)eyePos.y - 2, 4, 4};
    SDL_RenderFillRect(renderer, &eyeRect);
    eyeOffset.x = 10;
    eyePos = pixelPos + eyeOffset;
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
    
    // Set wind-up, animation durations and damage
    switch (attackType) {
        case BossAttackAnim::HORIZONTAL_SWING:
            m_windupDuration = 0.7f;
            m_animDuration = 0.4f;
            m_currentAttackDamage = m_baseAttackDamage;
            break;
        case BossAttackAnim::SPIN_ATTACK:
            m_windupDuration = 0.8f;
            m_animDuration = 0.8f;
            m_currentAttackDamage = m_baseAttackDamage * 1.2f;
            break;
        case BossAttackAnim::OVERHEAD_SWING:
            m_windupDuration = 0.9f;
            m_animDuration = 0.5f;
            m_currentAttackDamage = m_baseAttackDamage * 1.5f;
            break;
        case BossAttackAnim::UPPERCUT:
            m_windupDuration = 0.7f;
            m_animDuration = 0.4f;
            m_currentAttackDamage = m_baseAttackDamage * 1.3f;
            break;
        case BossAttackAnim::GROUND_SLAM:
            m_windupDuration = 1.0f;
            m_animDuration = 0.6f;
            m_currentAttackDamage = m_baseAttackDamage * 1.8f;
            break;
        case BossAttackAnim::DASH_ATTACK:
            m_windupDuration = 0.6f;  // Quick wind-up for dash
            m_animDuration = 0.4f;
            m_currentAttackDamage = m_baseAttackDamage;
            break;
        case BossAttackAnim::BACKSTEP_SLASH:
            m_windupDuration = 0.6f;
            m_animDuration = 0.4f;
            m_currentAttackDamage = m_baseAttackDamage * 1.1f;
            break;
        case BossAttackAnim::PROJECTILE:
            m_windupDuration = 1.0f;  // Long wind-up for projectile
            m_animDuration = 0.5f;
            m_currentAttackDamage = m_baseAttackDamage * 0.8f;
            break;
    }
    
    m_windupTimer = m_windupDuration;
    m_animTimer = m_animDuration;
}

void Boss::startMoving(const Vector2D& targetPos, float speedMultiplier) {
    if (!canAct()) return;
    
    m_animState = BossAnimState::MOVING;
    m_targetMovePosition = targetPos;
    m_currentMoveSpeed = m_moveSpeed * speedMultiplier;
    m_facingDirection = (targetPos - m_position).normalized();
}

void Boss::stopMoving() {
    if (m_animState == BossAnimState::MOVING) {
        m_animState = BossAnimState::IDLE;
    }
}

void Boss::performStep(const Vector2D& direction, float distance) {
    if (!canAct()) return;
    
    if (g_sifAI->isDebugEnabled()) {
        std::cout << "Step Distance: " << distance << std::endl;
    }
    Vector2D stepTarget = m_position + direction.normalized() * distance;
    
    // Keep in bounds
    float minX = GameUnits::toMeters(60.0f);  // 2.67 meters
    float maxX = GameUnits::toMeters(740.0f); // 24 meters
    float minY = GameUnits::toMeters(120.0f);  // 2.67 meters
    float maxY = GameUnits::toMeters(480.0f); // 17.33 meters
    stepTarget.x = std::max(minX, std::min(maxX, stepTarget.x));
    stepTarget.y = std::max(minY, std::min(maxY, stepTarget.y));

    // Use fast movement for steps
    startMoving(stepTarget, 2.0f);
}

// Cancel an ongoing attack (add to Boss.cpp)
void Boss::cancelAttack() {
    if (m_animState == BossAnimState::ATTACKING || 
        m_animState == BossAnimState::RECOVERING) {
        m_animState = BossAnimState::IDLE;
        m_animTimer = 0;
        m_animDuration = 0;
        m_windupTimer = 0;
        m_windupDuration = 0;
        m_hasDealtDamage = false;
        
        // Reset sword to idle position
        m_swordAngle = m_swordOnRightSide ? 0.0f : M_PI;
    }
}

// Force return to idle state
void Boss::forceIdle() {
    m_animState = BossAnimState::IDLE;
    m_animTimer = 0;
    m_animDuration = 0;
    m_windupTimer = 0;
    m_windupDuration = 0;
    m_hasDealtDamage = false;
    m_swordAngle = m_swordOnRightSide ? 0.0f : M_PI;
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
    m_swordBase = m_position + m_facingDirection * GameUnits::toMeters(30);
    m_swordTipPosition = m_swordBase + Vector2D(cos(m_swordAngle), sin(m_swordAngle)) * m_swordLength;
}

float Boss::getAnimationProgress() const {
    if (m_windupTimer > 0 && m_windupDuration > 0) {
        // Still in wind-up phase
        return 0.0f;
    }

    if (m_animDuration > 0) {
        return 1.0f - (m_animTimer / m_animDuration);
    }
    return 1.0f;
}

bool Boss::isAttacking() const {
    return m_animState == BossAnimState::ATTACKING && m_windupTimer <= 0;
}

bool Boss::isWindingUp() const {
    return m_animState == BossAnimState::ATTACKING && m_windupTimer > 0;
}

Circle Boss::getAttackCircle() const {
    if (m_currentAttackAnim == BossAttackAnim::SPIN_ATTACK) {
        return Circle(m_position.x, m_position.y, m_swordLength + 40);
    }
    return Circle(m_swordTipPosition.x, m_swordTipPosition.y, 30);
}

SDL_Rect Boss::getSwordHitbox() const {
    Vector2D pixelPos = GameUnits::toPixels(m_swordBase);
    Vector2D pixelSwordTip = GameUnits::toPixels(m_swordTipPosition);

    int x = std::min(pixelPos.x, pixelSwordTip.x) - 10;
    int y = std::min(pixelPos.y, pixelSwordTip.y) - 10;
    int w = std::abs(pixelSwordTip.x - pixelPos.x) + 20;
    int h = std::abs(pixelSwordTip.y - pixelPos.y) + 20;
    return SDL_Rect{x, y, w, h};
}
