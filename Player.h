#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "GameUnits.h"
#include "LTexture.h"

enum class PlayerState {
    IDLE,
    MOVING,
    ATTACKING,
    DODGING,
    // STUNNED,
    TAKING_DAMAGE,
    DYING,
    DEAD,
};

enum class PlayerDirection {
    DOWN = 0,   
    LEFT = 1,    
    RIGHT = 2,  
    UP = 3      
};

enum class WeaponType {
    NONE,
    SWORD,
    BOW
};

enum class AnimationType {
    // Basic animations
    IDLE = 0,
    RUN = 1,
    DASH = 2,
    TAKE_DAMAGE = 3,
    DEATH = 4,
    
    // Sword animations
    SWORD_IDLE = 5,
    SWORD_RUN = 6,
    SWORD_ATTACK1 = 7,
    // SWORD_ATTACK1_END = 8,
    SWORD_ATTACK2 = 9,
    // SWORD_COMBO = 10,
    // SWORD_SPECIAL = 11,
    
    // Bow animations  
    // BOW_IDLE = 12,
    // BOW_RUN = 13,
    // BOW_ATTACK1 = 14,
    // BOW_ATTACK1_END = 15,
    // BOW_ATTACK2 = 16,
    // BOW_COMBO = 17,
    // BOW_SPECIAL = 18
};

class Player : public Entity {
private:
    PlayerState m_state;
    PlayerDirection m_direction;
    WeaponType m_currentWeapon;
    AnimationType m_currentAnimation;

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
    int m_comboCount;

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

    // Sprite rendering
    static LTexture s_playerSpriteSheet;  // Shared texture for all players
    static bool s_textureLoaded;          // Track if texture is loaded

    SDL_Rect m_currentFrame;
    int m_frameIndex;
    float m_frameTime;
    float m_frameTimer;
    bool m_animationComplete;    

    // Sprite sheet layout constants
    static const int FRAMES_PER_ANIMATION = 6;  // 6 frames per animation
    static const int FRAME_WIDTH = 128;
    static const int FRAME_HEIGHT = 128;
    static const int DIRECTIONS_PER_ANIMATION = 4;  // 4 directions (if applicable)

    void updateSwordPosition();
    void updateAnimation(float deltaTime);
    void updateDirection(const Vector2D& moveDir);
    void setAnimation(AnimationType animation);
    AnimationType getIdleAnimation() const;
    AnimationType getRunAnimation() const;
    AnimationType getAttackAnimation() const;

public:
    Player(float x, float y);
    ~Player();

    // Static method to load shared texture
    static bool loadTexture(SDL_Renderer* renderer, const std::string& path);
    static void freeTexture();

    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    
    void move(const Vector2D& direction);
    void attack();
    void dodge(const Vector2D& direction);
    void setWindowBounds(float width, float height) { 
        m_windowWidth = GameUnits::toMeters(width); 
        m_windowHeight = GameUnits::toMeters(height); 
    }
    void takeDamage(float damage) override;
    
    bool canAttack() const;
    bool canDodge() const;
    bool isInvulnerable() const;
    bool isAttacking() const { return m_state == PlayerState::ATTACKING; }
    bool isAnimationComplete() const { return m_animationComplete; }
    
    float getAttackDamage() const { return m_attackDamage; }
    float getAttackRange() const { return m_attackRange; }
    float getStaminaPercentage() const { return m_currentStamina / m_maxStamina; }
    PlayerState getState() const { return m_state; }
    WeaponType getCurrentWeapon() const { return m_currentWeapon; }
    bool hasDealtDamage() const { return m_hasDealtDamage; }
    void setDamageDealt() { m_hasDealtDamage = true; }

    // Get sword hitbox for collision detection
    SDL_Rect getSwordHitbox() const;
    Circle getAttackCircle() const { return Circle(m_position.x, m_position.y, m_attackRange); }
};

#endif
