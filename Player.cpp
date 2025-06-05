#include "Player.h"
#include "GameUnits.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Player::Player(float x, float y)
    : Entity(x, y, 30, 50, 100),  // postion, width, height, health
      m_state(PlayerState::IDLE),
      m_speed(10.0f),
      m_maxStamina(100.0f),
      m_currentStamina(100.0f),
      m_staminaRegenRate(40.0f),
      m_staminaRegenDelay(0.4f),  
      m_timeSinceStaminaUse(0.0f),
      m_attackCooldown(0.0f),
      m_attackDamage(20.0f),
      m_attackRange(2.0f),
      m_hasDealtDamage(false),
      m_swordAngle(0.0f),
      m_swordLength(1.5f),
      m_facingDirection(0, -1),  
      m_dodgeCooldown(0.0f),
      m_dodgeDuration(0.5f),
      m_dodgeSpeed(10.0f),
      m_stateTimer(0.0f),
      m_animationTimer(0.0f),
      m_windowWidth(GameUnits::toMeters(800.0f)),
      m_windowHeight(GameUnits::toMeters(600.0f)) {
    updateSwordPosition();
}

void Player::update(float deltaTime) {
    // Update cooldowns
    if (m_attackCooldown > 0) m_attackCooldown -= deltaTime;
    if (m_dodgeCooldown > 0) m_dodgeCooldown -= deltaTime;
    
    m_animationTimer += deltaTime;

    std::cout << "Player State: " << static_cast<int>(m_state) << std::endl;

    // Regenerate stamina
    if (m_state == PlayerState::DODGING || m_state == PlayerState::ATTACKING) {
        m_timeSinceStaminaUse = 0.0f;
    } else {
        m_timeSinceStaminaUse += deltaTime;
    }

    if (m_timeSinceStaminaUse >= m_staminaRegenDelay) {
        m_currentStamina = std::min(m_maxStamina, m_currentStamina + m_staminaRegenRate * deltaTime);
    }
    
    // Update facing direction towards boss when not attacking or dodging
    // if (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) {
    //     Vector2D toBoss = (m_targetPosition - m_position);
    //     if (toBoss.length() > 0) {
    //         m_facingDirection = toBoss.normalized();
    //     }
    // }

    // Update facing direction based on movement (not boss position)
    if (m_state == PlayerState::MOVING && m_velocity.length() > 0) {
        m_facingDirection = m_velocity.normalized();
    }
    // When not moving, keep the last facing direction (don't auto-face boss)
 
    // Update state
    switch (m_state) {
        case PlayerState::ATTACKING:
            m_stateTimer -= deltaTime;
            // Animate sword swing
            m_swordAngle = -M_PI/3 + (M_PI * 2/3 * (0.3f - m_stateTimer) / 0.3f);
            if (m_stateTimer <= 0) {
                m_state = PlayerState::IDLE;
                // m_attackCooldown = 0.5f;
                m_swordAngle = 0;
            }
            break;
            
        case PlayerState::DODGING:
            m_stateTimer -= deltaTime;
            m_position = m_position + m_dodgeDirection * m_dodgeSpeed * deltaTime;
            if (m_stateTimer <= 0) {
                m_state = PlayerState::IDLE;
            }
            break;
            
        case PlayerState::MOVING:
            m_position = m_position + m_velocity * deltaTime;
            // Sword bob while moving
            m_swordAngle = sin(m_animationTimer * 3) * 0.15f;
            break;

        case PlayerState::IDLE:
            // Idle sword animation
            m_swordAngle = sin(m_animationTimer * 2) * 0.1f;
            break;
            
        default:
            break;
    }
    
    // Keep player within window bounds
    float halfWidth = m_width / 2;
    float halfHeight = m_height / 2;
    m_position.x = std::max(halfWidth, std::min(m_windowWidth - halfWidth, m_position.x));
    m_position.y = std::max(halfHeight, std::min(m_windowHeight - halfHeight, m_position.y));
    updateSwordPosition();
}

void Player::render(SDL_Renderer* renderer) {
    SDL_Color color;
    switch (m_state) {
        case PlayerState::ATTACKING:
            color = {255, 255, 0, 255}; // Yellow when attacking
            break;
        case PlayerState::DODGING:
            color = {100, 100, 255, 128}; // Semi-transparent blue when dodging
            break;
        default:
            color = {0, 255, 0, 255}; // Green normally
            break;
    }
    
    Vector2D pixelPos = GameUnits::toPixels(m_position);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = getCollisionBox();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw sword
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Silver sword
    
    // Calculate sword base position (held by the player)
    Vector2D swordBase = m_position + m_facingDirection * GameUnits::toMeters(15);
    
    // Calculate sword tip based on angle
    float totalAngle = atan2(m_facingDirection.y, m_facingDirection.x) + m_swordAngle;
    Vector2D swordEnd = swordBase + Vector2D(cos(totalAngle), sin(totalAngle)) * m_swordLength;

    Vector2D pixelBase = GameUnits::toPixels(swordBase);
    Vector2D pixelEnd = GameUnits::toPixels(swordEnd);

    // Draw sword as a thick line
    for (int i = -2; i <= 2; i++) {
        SDL_RenderDrawLine(renderer, 
            pixelBase.x + i, pixelBase.y,
            pixelEnd.x + i, pixelEnd.y);
        SDL_RenderDrawLine(renderer, 
            pixelBase .x, pixelBase.y + i,
            pixelEnd.x, pixelEnd.y + i);
    }
    
    // Draw sword hilt
    SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255); // Blue hilt
    SDL_Rect hiltRect = {
        (int)pixelBase.x - 4,
        (int)pixelBase.y - 4,
        8, 8
    };
    SDL_RenderFillRect(renderer, &hiltRect);
    
    // Draw eyes to show facing direction
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    Vector2D eyeOffset = m_facingDirection * 8;
    Vector2D eyeRight(-m_facingDirection.y, m_facingDirection.x);
    
    Vector2D eye1 = pixelPos + eyeOffset + eyeRight * 5;
    Vector2D eye2 = pixelPos + eyeOffset - eyeRight * 5;
    
    SDL_Rect eyeRect1 = {(int)eye1.x - 2, (int)eye1.y - 2, 3, 3};
    SDL_Rect eyeRect2 = {(int)eye2.x - 2, (int)eye2.y - 2, 3, 3};
    SDL_RenderFillRect(renderer, &eyeRect1);
    SDL_RenderFillRect(renderer, &eyeRect2);
}

void Player::move(const Vector2D& direction) {
    if (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) {
        if (direction.length() > 0) {
            m_velocity = direction.normalized() * m_speed;
            m_state = PlayerState::MOVING;
            m_facingDirection = direction.normalized();
        } else {
            m_velocity = Vector2D(0, 0);
            m_state = PlayerState::IDLE;
        }
    }
}

void Player::attack() {
    if (canAttack()) {
        m_state = PlayerState::ATTACKING;
        m_stateTimer = 0.3f;
        m_attackCooldown = 0.5f;
        m_currentStamina -= 15.0f;
        m_hasDealtDamage = false;  // Reset the flag for new attack
        m_swordAngle = -M_PI/3;  // Start position for swing
    }
}

void Player::dodge(const Vector2D& direction) {
    if (canDodge()) {
        m_state = PlayerState::DODGING;
        m_stateTimer = m_dodgeDuration;
        m_dodgeCooldown = 0.5f;
        m_dodgeDirection = direction.normalized();
        m_currentStamina -= 25.0f;
        // Face the dodge direction
        m_facingDirection = direction.normalized();
    }
}

bool Player::canAttack() const {
    return (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) && m_attackCooldown <= 0 && m_currentStamina >= 20.0f;
}

bool Player::canDodge() const {
    return (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) && 
           m_dodgeCooldown <= 0 && m_currentStamina >= 25.0f;
}

bool Player::isInvulnerable() const {
    return m_state == PlayerState::DODGING;
}

void Player::updateSwordPosition() {
    float angle = atan2(m_facingDirection.y, m_facingDirection.x) + m_swordAngle;
    Vector2D swordBase = m_position + m_facingDirection * GameUnits::toMeters(15);
    m_swordTipPosition = swordBase + Vector2D(cos(angle), sin(angle)) * m_swordLength;
}

SDL_Rect Player::getSwordHitbox() const {
    // Create a rectangle along the sword's length
    Vector2D swordBase = m_position + m_facingDirection * GameUnits::toMeters(15);
    Vector2D pixelPos = GameUnits::toPixels(swordBase);
    Vector2D pixelSwordTip = GameUnits::toPixels(m_swordTipPosition);

    int x = (int)(std::min(pixelPos.x, pixelSwordTip.x) - 10);
    int y = (int)(std::min(pixelPos.y, pixelSwordTip.y) - 10);
    int w = (int)(std::abs(pixelSwordTip.x - pixelPos.x) + 20);
    int h = (int)(std::abs(pixelSwordTip.y - pixelPos.y) + 20);

    return SDL_Rect{x, y, w, h};
}
