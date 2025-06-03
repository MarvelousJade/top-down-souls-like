#pragma once
#include <SDL2/SDL.h>
#include <random>
#include <memory>
#include <vector>
#include <cmath>
#include "Entity.h"
#include "Goals.h"
#include "Player.h"
#include "Vector2D.h"
#include "Boss.h"

// Main AI class
class HolySwordWolfAI {
    Boss* m_self;
    Player* m_target;
    std::mt19937 m_rng;
    
    // AI state
    std::vector<std::unique_ptr<AIGoal>> m_goalQueue;
    std::unique_ptr<AIGoal> m_currentGoal;
    
    // Special states
    bool m_isEnhanced = false; // Special effect 5401 in the scripts
    float m_enhancedTimer = 0;
    
    // Combat state
    float m_lastDamageTime = 0;
    bool m_isGuardBroken = false;
    
    // Helper variables
    float m_actionCooldown = 0;
    int m_aggressionLevel = 0; // Tracks from script's ai:GetNumber(0)
    
    // Distance thresholds
    const float ATTACK_CLOSE = 80.0f;
    const float ATTACK_MID = 120.0f;
    const float ATTACK_FAR = 300.0f;
    
public:
    HolySwordWolfAI(Boss* entity, Player* player);
    
    void update(float deltaTime);
    void onDamaged(float damage, const Vector2D& sourcePos);
    void onGuardBroken();
    void onProjectileDetected(const Vector2D& projectilePos);
    
    // Core AI functions
    void selectAction();
    void executeGoal(std::unique_ptr<AIGoal> goal);
    void clearGoals();
    void addGoal(std::unique_ptr<AIGoal> goal);
    
    // Utility functions
    float getDistanceToTarget() const;
    float getAngleToTarget() const;
    bool isTargetBehind() const;
    bool isTargetOnSide(bool checkRight) const;
    float getTargetHPRate() const;
    float getSelfHPRate() const;
    int getRandomInt(int min, int max);
    float getRandomFloat(float min, float max);
    
    // State checks
    bool isEnhanced() const { return m_isEnhanced; }
    void setEnhanced(bool enhanced) { m_isEnhanced = enhanced; }
    
    // Movement functions for goals
    void moveToward(const Vector2D& pos, float speed);
    void moveAway(const Vector2D& pos, float speed);
    void moveSideways(bool right, float speed);
    void performStep(StepType type, float distance);
    
    // Attack functions for goals
    void startAttack(AttackType type);
    bool isAttacking() const;
    
    // Friend access for goals
    friend class AttackGoal;
    friend class MoveToTargetGoal;
    friend class StepGoal;
    friend class SidewayMoveGoal;
};
