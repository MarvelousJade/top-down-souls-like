#ifndef GOALS_H
#define GOALS_H

class HolySwordWolfAI;

// Attack types based on the Lua scripts
enum class AttackType {
    LIGHT_COMBO_1 = 3000,  // Close range light attack combo
    LIGHT_COMBO_2 = 3001,
    LIGHT_COMBO_3 = 3002,
    DASH_ATTACK = 3003,    // Mid-range dash attack
    DASH_FOLLOWUP = 3004,
    SPIN_ATTACK = 3005,    // Close range spin
    UPPERCUT = 3006,       // Close range uppercut
    BACKSTEP_SLASH_R = 3007, // Counter attacks
    BACKSTEP_SLASH_L = 3008,
    GROUND_SLAM = 3009,    // Close range slam
    GROUND_SLAM_FOLLOWUP = 3010,
    PROJECTILE = 3011,     // Long range projectile
    
    // Enhanced state attacks
    ENHANCED_COMBO_1 = 13000,
    ENHANCED_COMBO_2 = 13002,
    ENHANCED_SPIN_R = 13003,
    ENHANCED_SPIN_L = 13004,
};

enum class StepType {
    BACKSTEP = 701,
    SIDESTEP_LEFT = 702,
    SIDESTEP_RIGHT = 703,
};

enum class GoalType {
    ATTACK,
    MOVE_TO_TARGET,
    STEP,
    SIDEWAY_MOVE
};

// Base class for AI Goals
class AIGoal {
public:
    virtual ~AIGoal() = default;
    virtual void activate(class HolySwordWolfAI* ai) = 0;
    virtual bool update(class HolySwordWolfAI* ai, float deltaTime) = 0; // Returns true when complete
    virtual void terminate(class HolySwordWolfAI* ai) = 0;
    virtual GoalType getType() const = 0;
    
    float lifeTime = -1.0f; // -1 means infinite
};

// Specific goal implementations
class AttackGoal : public AIGoal {
private:
    AttackType attackType;
    float chargeTime;
    float currentTime = 0;
    
public:
    AttackGoal(AttackType type, float charge = 0.5f) 
        : attackType(type), chargeTime(charge) {}
    
    void activate(class HolySwordWolfAI* ai) override;
    bool update(class HolySwordWolfAI* ai, float deltaTime) override;
    void terminate(class HolySwordWolfAI* ai) override;
    GoalType getType() const override { return GoalType::ATTACK; }
    AttackType getAttackType() const { return attackType; }
};

class MoveToTargetGoal : public AIGoal {
private:
    float targetDistance;
    bool walk;
    
public:
    MoveToTargetGoal(float dist, bool shouldWalk = false) 
        : targetDistance(dist), walk(shouldWalk) {}
    
    void activate(class HolySwordWolfAI* ai) override;
    bool update(class HolySwordWolfAI* ai, float deltaTime) override;
    void terminate(class HolySwordWolfAI* ai) override;
    GoalType getType() const override { return GoalType::MOVE_TO_TARGET; }
    float getTargetDistance() const { return targetDistance; }
};

class StepGoal : public AIGoal {
private:
    StepType stepType;
    float stepDistance;
    float currentProgress = 0;
    
public:
    StepGoal(StepType type, float dist = 5.0f) 
        : stepType(type), stepDistance(dist) {}
    
    void activate(class HolySwordWolfAI* ai) override;
    bool update(class HolySwordWolfAI* ai, float deltaTime) override;
    void terminate(class HolySwordWolfAI* ai) override;
    GoalType getType() const override { return GoalType::STEP; }
    StepType getStepType() const { return stepType; }
};

class SidewayMoveGoal : public AIGoal {
private:
    bool moveRight;
    float duration;
    float currentTime = 0;
    
public:
    SidewayMoveGoal(bool right, float dur) 
        : moveRight(right), duration(dur) {}
    
    void activate(class HolySwordWolfAI* ai) override;
    bool update(class HolySwordWolfAI* ai, float deltaTime) override;
    void terminate(class HolySwordWolfAI* ai) override;
    GoalType getType() const override { return GoalType::SIDEWAY_MOVE; }
    bool isMoveRight() const { return moveRight; }
};

#endif
