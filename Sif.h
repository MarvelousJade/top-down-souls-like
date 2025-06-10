#ifndef SIF_H
#define SIF_H

#include <SDL2/SDL.h>
#include <random>
#include <memory>
#include <vector>
#include <cmath>
#include <deque>
#include "Entity.h"
#include "Goals.h"
#include "Player.h"
#include "Vector2D.h"
#include "Boss.h"

// Debug information for goal tracking
struct GoalDebugInfo {
    std::string goalName;
    std::string reason;
    float timestamp;
    GoalType type;
};

// Main AI class
class HolySwordWolfAI {
    Boss* m_self;
    Player* m_target;
    std::mt19937 m_rng;
    
    // AI state
    std::vector<std::unique_ptr<AIGoal>> m_goalQueue;
    std::unique_ptr<AIGoal> m_currentGoal;
    
    // Debug system
    bool m_debugEnabled = false;
    std::deque<GoalDebugInfo> m_goalHistory;  // Recent goal additions
    std::vector<std::string> m_goalQueueDebug;  // Current queue state
    std::string m_currentGoalDebug = "None";
    std::string m_lastActionReason = "";
    float m_debugTimer = 0;
    static const size_t MAX_HISTORY_SIZE = 20;

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
    const float ATTACK_VERY_CLOSE = 4.0f ; // 4
    const float ATTACK_CLOSE = bossAttackRange;
    const float ATTACK_MID = 8.0f;
    const float ATTACK_FAR = 12.0f;

    // Debug helper functions
    std::string goalTypeToString(GoalType type) const;
    std::string attackTypeToString(AttackType type) const;
    std::string stepTypeToString(StepType type) const;
    void logGoalAddition(const std::string& goalName, const std::string& reason);
    void updateGoalQueueDebug();
    
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
    void addGoalWithReason(std::unique_ptr<AIGoal> goal, const std::string& reason);
    
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
    
    // Debug system
    void setDebugEnabled(bool enabled) { m_debugEnabled = enabled; }
    bool isDebugEnabled() const { return m_debugEnabled; }
    const std::deque<GoalDebugInfo>& getGoalHistory() const { return m_goalHistory; }
    const std::vector<std::string>& getGoalQueueDebug() const { return m_goalQueueDebug; }
    const std::string& getCurrentGoalDebug() const { return m_currentGoalDebug; }
    int getAggressionLevel() const { return m_aggressionLevel; }
    float getActionCooldown() const { return m_actionCooldown; }

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

extern HolySwordWolfAI* g_sifAI;

#endif
