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
    
    // Distance thresholds from the scripts
    const float ATTACK_CLOSE = 2.0f;
    const float ATTACK_MID = 5.2f;
    const float ATTACK_FAR = 13.5f;
    
public:
    HolySwordWolfAI(Boss* entity, Player* player);
    
    void Update(float deltaTime);
    void OnDamaged(float damage, const Vector2D& sourcePos);
    void OnGuardBroken();
    void OnProjectileDetected(const Vector2D& projectilePos);
    
    // Core AI functions
    void SelectAction();
    void ExecuteGoal(std::unique_ptr<AIGoal> goal);
    void ClearGoals();
    void AddGoal(std::unique_ptr<AIGoal> goal);
    
    // Utility functions
    float GetDistanceToTarget() const;
    float GetAngleToTarget() const;
    bool IsTargetBehind() const;
    bool IsTargetOnSide(bool checkRight) const;
    float GetTargetHPRate() const;
    float GetSelfHPRate() const;
    int GetRandomInt(int min, int max);
    float GetRandomFloat(float min, float max);
    
    // State checks
    bool IsEnhanced() const { return m_isEnhanced; }
    void SetEnhanced(bool enhanced) { m_isEnhanced = enhanced; }
    
    // Movement functions for goals
    void MoveToward(const Vector2D& pos, float speed);
    void MoveAway(const Vector2D& pos, float speed);
    void MoveSideways(bool right, float speed);
    void PerformStep(StepType type, float distance);
    
    // Attack functions for goals
    void StartAttack(AttackType type);
    bool IsAttacking() const;
    
    // Friend access for goals
    friend class AttackGoal;
    friend class MoveToTargetGoal;
    friend class StepGoal;
    friend class SidewayMoveGoal;
};
