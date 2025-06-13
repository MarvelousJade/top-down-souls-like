#include "Player.h"
#include "GameUnits.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static member definitions
LTexture Player::s_playerSpriteSheet;
bool Player::s_textureLoaded = false;

Player::Player(float x, float y)
    : Entity(x, y, 30, 50, 100),  // postion, width, height, health
      m_state(PlayerState::IDLE),
      m_direction(PlayerDirection::DOWN),
      m_currentWeapon(WeaponType::SWORD),
      m_currentAnimation(AnimationType::SWORD_IDLE),
      m_speed(10.0f),
      m_maxStamina(100.0f),
      m_currentStamina(100.0f),
      m_staminaRegenRate(60.0f),
      m_staminaRegenDelay(0.4f),  
      m_timeSinceStaminaUse(0.0f),
      m_attackCooldown(0.0f),
      m_attackDamage(20.0f),
      m_attackRange(3.0f),
      m_hasDealtDamage(false),
      m_comboCount(0),
      m_swordAngle(0.0f),
      m_swordLength(2.5f),
      m_facingDirection(0, -1),  
      m_dodgeCooldown(0.0f),
      m_dodgeDuration(0.5f),
      m_dodgeSpeed(10.0f),
      m_dodgeDirection(0, -1),
      m_stateTimer(0.0f),
      m_animationTimer(0.0f),
      m_windowWidth(GameUnits::toMeters(800.0f)),
      m_windowHeight(GameUnits::toMeters(600.0f)),
      m_frameIndex(0),
      m_frameTime(0.2f),  // 200ms per frame
      m_frameTimer(0.0f),
      m_animationComplete(true) {

    // Initialize first frame (idle, facing down)
    m_currentFrame = {
        0,  // x position in sprite sheet
        0,  // y position in sprite sheet  
        FRAME_WIDTH,
        FRAME_HEIGHT
    };

    updateSwordPosition();
}


Player::~Player() {
    // Destructor - static texture is cleaned up separately
}

bool Player::loadTexture(SDL_Renderer* renderer, const std::string& path) {
    if (s_textureLoaded) {
        return true;  // Already loaded
    }
    
    bool success = s_playerSpriteSheet.loadFromFile(path);
    if (success) {
        s_textureLoaded = true;
        
        // Debug: Check sprite sheet dimensions
        int width = s_playerSpriteSheet.getWidth();
        int height = s_playerSpriteSheet.getHeight();
        int expectedWidth = FRAME_WIDTH * FRAMES_PER_ANIMATION * DIRECTIONS_PER_ANIMATION; // 128 * 6 * 4 = 3072
        int expectedHeight = FRAME_HEIGHT * 7; // 128 * 7 = 896
        
        std::cout << "Player sprite sheet loaded: " << path << std::endl;
        std::cout << "  Actual dimensions: " << width << "x" << height << std::endl;
        std::cout << "  Expected dimensions: " << expectedWidth << "x" << expectedHeight << std::endl;
        
        if (width != expectedWidth || height != expectedHeight) {
            std::cerr << "WARNING: Sprite sheet dimensions don't match expected layout!" << std::endl;
            std::cerr << "  This may cause rendering issues or crashes." << std::endl;
        } else {
            std::cout << "  Sprite sheet dimensions are correct!" << std::endl;
        }
        
    } else {
        std::cerr << "Failed to load player sprite sheet: " << path << std::endl;
    }
    return success;
}

void Player::freeTexture() {
    if (s_textureLoaded) {
        s_playerSpriteSheet.free();
        s_textureLoaded = false;
    }
}

void Player::setAnimation(AnimationType animation) {
    if (m_currentAnimation != animation) {
        m_currentAnimation = animation;
        m_frameIndex = 0;
        m_frameTimer = 0.0f;
        m_animationComplete = false;
        
        // Set frame time based on animation type
        switch (animation) {
            case AnimationType::SWORD_ATTACK1:
            case AnimationType::SWORD_ATTACK2:
                m_frameTime = 0.1f;  // Fast attack animations
                break;
            case AnimationType::DASH:
                m_frameTime = 0.08f;  // Very fast dash
                break;
            case AnimationType::TAKE_DAMAGE:
            case AnimationType::DEATH:
                m_frameTime = 0.15f;  // Slower for impact
                break;
            default:
                m_frameTime = 0.2f;   // Normal speed
                break;
        }
    }
}

void Player::updateAnimation(float deltaTime) {
    m_frameTimer += deltaTime;
    
    if (m_frameTimer >= m_frameTime) {
        m_frameTimer = 0.0f;
        m_frameIndex++;
        
        // Check if animation should loop or complete
        bool shouldLoop = true;
        switch (m_currentAnimation) {
            case AnimationType::SWORD_ATTACK1:
            // case AnimationType::SWORD_ATTACK1_END:
            case AnimationType::SWORD_ATTACK2:
            // case AnimationType::BOW_ATTACK1:
            // case AnimationType::BOW_ATTACK1_END:
            // case AnimationType::BOW_ATTACK2:
            // case AnimationType::SWORD_SPECIAL:
            // case AnimationType::BOW_SPECIAL:
            case AnimationType::TAKE_DAMAGE:
            case AnimationType::DEATH:
            case AnimationType::DASH:
                shouldLoop = false;  // These animations play once
                break;
            default:
                shouldLoop = true;   // Idle, run animations loop
                break;
        }
        
        if (m_frameIndex >= FRAMES_PER_ANIMATION) {
            if (shouldLoop) {
                m_frameIndex = 0;  // Loop back to start
            } else {
                m_frameIndex = FRAMES_PER_ANIMATION - 1;  // Stay on last frame
                m_animationComplete = true;
            }
        }
    }
    
    // Calculate the source rectangle for current frame
    // Assuming sprite sheet layout: [Animation_Row][Direction_Column * FRAMES_PER_ANIMATION + Frame]
    int animationRow = static_cast<int>(m_currentAnimation);
    int directionOffset = static_cast<int>(m_direction);

    // Bounds checking
    if (animationRow < 0 || animationRow >= TOTAL_ANIMATIONS) animationRow = 0;
    if (directionOffset < 0 || directionOffset >= DIRECTIONS_PER_ANIMATION) directionOffset = 0;
    if (m_frameIndex < 0 || m_frameIndex >= FRAMES_PER_ANIMATION) m_frameIndex = 0;
    
    // For directional animations (most animations have 4 directions)
    bool isDirectional = true; // m_currentAnimation != AnimationType::DEATH;
    
    // Calculate the full frame position
    int baseX, baseY;
    if (isDirectional) {
        baseX = (directionOffset * FRAMES_PER_ANIMATION + m_frameIndex) * FRAME_WIDTH;
        baseY = animationRow * FRAME_HEIGHT;
    } else {
        // Non-directional animations (special attacks, death)
        m_currentFrame.x = m_frameIndex * FRAME_WIDTH;
        m_currentFrame.y = animationRow * FRAME_HEIGHT;
    }
    
    // NEW: Apply cropping to remove empty space
    m_currentFrame.x = baseX + CHAR_CROP_X;           // Offset into the frame
    m_currentFrame.y = baseY + CHAR_CROP_Y;           // Offset into the frame  
    m_currentFrame.w = CHAR_CROP_WIDTH;               // Smaller width
    m_currentFrame.h = CHAR_CROP_HEIGHT;              // Smaller height
}

AnimationType Player::getIdleAnimation() const {
    switch (m_currentWeapon) {
        case WeaponType::SWORD:
            return AnimationType::SWORD_IDLE;
        default:
            return AnimationType::SWORD_IDLE;
    }
}

AnimationType Player::getRunAnimation() const {
    switch (m_currentWeapon) {
        case WeaponType::SWORD:
            return AnimationType::SWORD_RUN;
        default:
            return AnimationType::SWORD_RUN;
    }
}

AnimationType Player::getAttackAnimation() const {
    switch (m_currentWeapon) {
        case WeaponType::SWORD:
            if (m_comboCount == 1) {
                return AnimationType::SWORD_ATTACK2;
            } else if (m_comboCount == 2) {
                return AnimationType::SWORD_ATTACK1;
            }

        default:
            return AnimationType::SWORD_ATTACK1;  // Default to basic attack
    }
}

void Player::updateDirection(const Vector2D& moveDir) {
    // Don't change direction if not moving
    if (moveDir.length() < 0.1f) {
        return; // Keep current direction
    }
    
    // Check which axis has stronger movement
    if (abs(moveDir.x) > abs(moveDir.y)) {
        // Horizontal movement is stronger
        if (moveDir.x > 0) {
            m_direction = PlayerDirection::RIGHT;
        } else {
            m_direction = PlayerDirection::LEFT;
        }
    } else {
        // Vertical movement is stronger (or equal)
        if (moveDir.y > 0) {
            m_direction = PlayerDirection::DOWN;
        } else {
            m_direction = PlayerDirection::UP;
        }
    }
}

void Player::update(float deltaTime) {
    // Update cooldowns
    if (m_attackCooldown > 0) m_attackCooldown -= deltaTime;
    if (m_dodgeCooldown > 0) m_dodgeCooldown -= deltaTime;
    
    m_animationTimer += deltaTime;

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
    // switch (m_state) {
    //     case PlayerState::ATTACKING:
    //         m_stateTimer -= deltaTime;
    //         // Animate sword swing
    //         m_swordAngle = -M_PI/3 + (M_PI * 2/3 * (0.3f - m_stateTimer) / 0.3f);
    //         if (m_stateTimer <= 0) {
    //             m_state = PlayerState::IDLE;
    //             // m_attackCooldown = 0.5f;
    //             m_swordAngle = 0;
    //         }
    //         break;
    //
    //     case PlayerState::DODGING:
    //         m_stateTimer -= deltaTime;
    //         m_position = m_position + m_dodgeDirection * m_dodgeSpeed * deltaTime;
    //         if (m_stateTimer <= 0) {
    //             m_state = PlayerState::IDLE;
    //         }
    //         break;
    //
    //     case PlayerState::MOVING:
    //         m_position = m_position + m_velocity * deltaTime;
    //         // Sword bob while moving
    //         m_swordAngle = sin(m_animationTimer * 3) * 0.15f;
    //         break;
    //
    //     case PlayerState::IDLE:
    //         // Idle sword animation
    //         m_swordAngle = sin(m_animationTimer * 2) * 0.1f;
    //         break;
    //
    //     default:
    //         break;
    // }

    // Handle state transitions and animations
    switch (m_state) {
        case PlayerState::IDLE:
            setAnimation(getIdleAnimation());
            break;
            
        case PlayerState::MOVING:
            setAnimation(getRunAnimation());
            m_position = m_position + m_velocity * deltaTime;
            break;
            
        case PlayerState::ATTACKING:
            m_stateTimer -= deltaTime;
            // Set attack animation if not already set
            if (m_currentAnimation != getAttackAnimation()) {
                setAnimation(getAttackAnimation());
            }
            
            // Check if attack animation is complete
            if (m_animationComplete) {
                m_state = PlayerState::IDLE;
                m_hasDealtDamage = false;
            }
            break;
            
        case PlayerState::DODGING:
            setAnimation(AnimationType::DASH);
            m_stateTimer -= deltaTime;
            m_position = m_position + m_dodgeDirection * m_dodgeSpeed * deltaTime;
            
            if (m_stateTimer <= 0 || m_animationComplete) {
                m_state = PlayerState::IDLE;
            }
            break;
            
        case PlayerState::TAKING_DAMAGE:
            setAnimation(AnimationType::TAKE_DAMAGE);
            if (m_animationComplete) {
                m_state = PlayerState::IDLE;
            }
            break;
            
        case PlayerState::DYING:
            setAnimation(AnimationType::DEATH);
            // Stay in dying state until animation completes
            if (m_animationComplete) {
                m_state = PlayerState::DEAD;
            }
            break;
            
        case PlayerState::DEAD:
            // Stay on last frame of death animation
            break;
    }
    
    updateAnimation(deltaTime);

    // Keep player within window bounds
    float halfWidth = m_width / 2;
    float halfHeight = m_height / 2;
    m_position.x = std::max(halfWidth, std::min(m_windowWidth - halfWidth, m_position.x));
    m_position.y = std::max(halfHeight, std::min(m_windowHeight - halfHeight, m_position.y));

    updateSwordPosition();
}

void Player::debugSizes() {
    std::cout << "=== PLAYER SIZE DEBUG ===" << std::endl;
    
    // Entity logical size (in meters)
    std::cout << "Entity size (meters): " << m_width << " x " << m_height << std::endl;
    
    // Entity size in pixels
    float widthPixels = GameUnits::toPixels(m_width);
    float heightPixels = GameUnits::toPixels(m_height);
    std::cout << "Entity size (pixels): " << widthPixels << " x " << heightPixels << std::endl;
    
    // Sprite frame size
    std::cout << "Sprite frame size: " << FRAME_WIDTH << " x " << FRAME_HEIGHT << std::endl;
    
    // Position
    Vector2D pixelPos = GameUnits::toPixels(m_position);
    std::cout << "Position (pixels): " << pixelPos.x << ", " << pixelPos.y << std::endl;
    
    // Render position
    int renderX = (int)pixelPos.x - FRAME_WIDTH / 2;
    int renderY = (int)pixelPos.y - FRAME_HEIGHT / 2;
    std::cout << "Render position: " << renderX << ", " << renderY << std::endl;
    
    // Collision box
    SDL_Rect collisionBox = getCollisionBox();
    std::cout << "Collision box: (" << collisionBox.x << ", " << collisionBox.y 
              << ", " << collisionBox.w << ", " << collisionBox.h << ")" << std::endl;
    
    std::cout << "======================" << std::endl;
}

void Player::render(SDL_Renderer* renderer) {
    static bool debugPrinted = false;
    if (!debugPrinted) {
        debugSizes();
        debugPrinted = true;
    }

    Vector2D pixelPos = GameUnits::toPixels(m_position);
    
    // Calculate render position (center the sprite on the entity position)
    int renderX = (int)pixelPos.x - FRAME_WIDTH / 2;
    int renderY = (int)pixelPos.y - FRAME_HEIGHT / 2;
    
    if (s_textureLoaded) {
        float spriteScale = 2.5f; // Adjust as needed
        int renderWidth = (int)(CHAR_CROP_WIDTH * spriteScale);
        int renderHeight = (int)(CHAR_CROP_HEIGHT * spriteScale);
        
        int renderX = (int)pixelPos.x - renderWidth / 2;
        int renderY = (int)pixelPos.y - renderHeight / 2;
        
        // Apply color modulation based on state
        switch (m_state) {
            case PlayerState::ATTACKING:
                s_playerSpriteSheet.setColor(255, 255, 150);  // Yellowish tint
                break;
            case PlayerState::DODGING:
                s_playerSpriteSheet.setColor(150, 150, 255);  // Bluish tint
                s_playerSpriteSheet.setAlpha(180);  // Semi-transparent
                break;
            default:
                s_playerSpriteSheet.setColor(255, 255, 255);  // Normal color
                s_playerSpriteSheet.setAlpha(255);  // Fully opaque
                break;
        }
        
        // Render the sprite
        // s_playerSpriteSheet.render(renderX, renderY, &m_currentFrame);
        
        // Render the CROPPED sprite (no empty space)
        SDL_Rect destRect = {renderX, renderY, renderWidth, renderHeight};
        SDL_RenderCopy(renderer, s_playerSpriteSheet.getTexture(), &m_currentFrame, &destRect);

        // Reset color modulation
        s_playerSpriteSheet.setColor(255, 255, 255);
        s_playerSpriteSheet.setAlpha(255);
    } else {
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
    }
    
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
}

void Player::move(const Vector2D& direction) {
    if (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) {
        if (direction.length() > 0) {
            m_velocity = direction.normalized() * m_speed;
            m_state = PlayerState::MOVING;
            m_facingDirection = direction.normalized();
            updateDirection(direction);
        } else {
            m_velocity = Vector2D(0, 0);
            m_state = PlayerState::IDLE;
        }
    }
}

void Player::attack() {
    if (canAttack()) {
        m_state = PlayerState::ATTACKING;
        m_stateTimer = 0.6f;
        m_attackCooldown = 0.7f;
        m_currentStamina -= 20.0f;
        m_hasDealtDamage = false;  // Reset the flag for new attack
        m_swordAngle = -M_PI/3;  // Start position for swing
    }
}

void Player::takeDamage(float damage) {
    if (m_state != PlayerState::TAKING_DAMAGE && m_state != PlayerState::DYING && !isInvulnerable()) {
        Entity::takeDamage(damage);  // Call base class method
        
        if (m_currentHealth <= 0) {
            m_state = PlayerState::DYING;
        } else {
            m_state = PlayerState::TAKING_DAMAGE;
            m_stateTimer = 0.3f;  // Brief damage state
        }
    }
}

void Player::dodge(const Vector2D& direction) {
    if (canDodge()) {
        m_state = PlayerState::DODGING;
        m_stateTimer = m_dodgeDuration;
        m_dodgeCooldown = 0.5f;
        m_dodgeDirection = direction.normalized();
        m_currentStamina -= 30.0f;
        // Face the dodge direction
        m_facingDirection = direction.normalized();
        updateDirection(direction); 
    }
}

bool Player::canAttack() const {
    return (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) && 
           m_attackCooldown <= 0 && m_currentStamina >= 20.0f;
}


bool Player::canDodge() const {
    return (m_state == PlayerState::IDLE || m_state == PlayerState::MOVING) && 
           m_dodgeCooldown <= 0 && m_currentStamina >= 0.0f;
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
