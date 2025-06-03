#include "Sif.h"
#include "Vector2D.h"
#include <random>
#include <iostream>


// Updated Goal implementations to work with Boss
void AttackGoal::activate(HolySwordWolfAI* ai) {
    currentTime = 0;
    
    // Map AttackType to BossAttackAnim
    BossAttackAnim bossAnim;
    switch (attackType) {
        case AttackType::LIGHT_COMBO_1:
        case AttackType::LIGHT_COMBO_2:
        case AttackType::LIGHT_COMBO_3:
        case AttackType::ENHANCED_COMBO_1:
            bossAnim = BossAttackAnim::HORIZONTAL_SWING;
            break;
            
        case AttackType::SPIN_ATTACK:
        case AttackType::ENHANCED_SPIN_R:
        case AttackType::ENHANCED_SPIN_L:
            bossAnim = BossAttackAnim::SPIN_ATTACK;
            break;
            
        case AttackType::UPPERCUT:
            bossAnim = BossAttackAnim::UPPERCUT;
            break;
            
        case AttackType::BACKSTEP_SLASH_R:
        case AttackType::BACKSTEP_SLASH_L:
            bossAnim = BossAttackAnim::BACKSTEP_SLASH;
            break;
            
        case AttackType::GROUND_SLAM:
        case AttackType::GROUND_SLAM_FOLLOWUP:
        case AttackType::ENHANCED_COMBO_2:
            bossAnim = BossAttackAnim::GROUND_SLAM;
            break;
            
        case AttackType::DASH_ATTACK:
        case AttackType::DASH_FOLLOWUP:
            bossAnim = BossAttackAnim::DASH_ATTACK;
            break;
            
        case AttackType::PROJECTILE:
            bossAnim = BossAttackAnim::PROJECTILE;
            break;
            
        default:
            bossAnim = BossAttackAnim::HORIZONTAL_SWING;
            break;
    }
    
    ai->m_self->startAttackAnimation(bossAnim);
}

bool AttackGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    currentTime += deltaTime;
    
    // Wait for attack animation to finish
    if (!ai->m_self->isAttacking() && !ai->m_self->isRecovering()) {
        return true; // Attack complete
    }
    
    return false;
}

void MoveToTargetGoal::activate(HolySwordWolfAI* ai) {
    // Calculate target position based on desired distance
    Vector2D toTarget = ai->m_target->getPosition() - ai->m_self->getPosition();
    float currentDist = toTarget.length();
    
    if (std::abs(currentDist - targetDistance) > 10.0f) {
        Vector2D targetPos;
        if (currentDist > targetDistance) {
            // Move closer
            targetPos = ai->m_self->getPosition() + toTarget.normalized() * (currentDist - targetDistance);
        } else {
            // Move away
            targetPos = ai->m_self->getPosition() - toTarget.normalized() * (targetDistance - currentDist);
        }
        
        float speed = walk ? 0.5f : 1.0f;
        ai->m_self->startMoving(targetPos, speed);
    }
}

bool MoveToTargetGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    float currentDist = ai->getDistanceToTarget();
    
    if (std::abs(currentDist - targetDistance) < 15.0f) {
        ai->m_self->stopMoving();
        return true;
    }
    
    // Check if still moving
    if (!ai->m_self->isAttacking() && !ai->m_self->isRecovering()) {
        // Recalculate if needed
        activate(ai);
    }
    
    return false;
}

void StepGoal::activate(HolySwordWolfAI* ai) {
    currentProgress = 0;
    
    Vector2D stepDir;
    Vector2D toTarget = (ai->m_target->getPosition() - ai->m_self->getPosition()).normalized();
    
    switch (stepType) {
        case StepType::BACKSTEP:
            stepDir = toTarget * -1; // Away from target
            break;
        case StepType::SIDESTEP_LEFT:
            stepDir = Vector2D(-toTarget.y, toTarget.x); // Perpendicular left
            break;
        case StepType::SIDESTEP_RIGHT:
            stepDir = Vector2D(toTarget.y, -toTarget.x); // Perpendicular right
            break;
    }
    
    ai->m_self->performStep(stepDir, stepDistance * 20.0f); // Scale up distance
}

bool StepGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    currentProgress += deltaTime;
    return currentProgress >= 0.3f; // Quick step
}

void SidewayMoveGoal::activate(HolySwordWolfAI* ai) {
    currentTime = 0;
    
    Vector2D toTarget = (ai->m_target->getPosition() - ai->m_self->getPosition()).normalized();
    Vector2D sideDir = moveRight ? Vector2D(toTarget.y, -toTarget.x) : Vector2D(-toTarget.y, toTarget.x);
    
    Vector2D targetPos = ai->m_self->getPosition() + sideDir * 100.0f;
    ai->m_self->startMoving(targetPos, 0.8f);
}

bool SidewayMoveGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    currentTime += deltaTime;
    if (currentTime >= duration) {
        ai->m_self->stopMoving();
        return true;
    }
    return false;
}

HolySwordWolfAI::HolySwordWolfAI(Boss* entity, Player* player)
    : m_self(entity), m_target(player), m_rng(std::random_device{}()),
      m_isEnhanced(false), m_enhancedTimer(0), m_lastDamageTime(0),
      m_isGuardBroken(false), m_actionCooldown(0), m_aggressionLevel(0) {
    // Additional initialization can be added here if needed
}

// Main AI Update
void HolySwordWolfAI::update(float deltaTime) {
    // Update boss facing direction
    if (m_self->canAct()) {
        m_self->setFacingDirection((m_target->getPosition() - m_self->getPosition()).normalized());
    }
    
    // Update timers
    if (m_actionCooldown > 0) {
        m_actionCooldown -= deltaTime;
    }
    
    if (m_isEnhanced) {
        m_enhancedTimer += deltaTime;
        if (m_enhancedTimer > 30.0f) {
            m_isEnhanced = false;
            m_enhancedTimer = 0;
        }
    }
    
    // Process current goal
    if (m_currentGoal) {
        if (m_currentGoal->update(this, deltaTime)) {
            m_currentGoal->terminate(this);
            m_currentGoal.reset();
        }
    }
    
    // Get next goal from queue
    if (!m_currentGoal && !m_goalQueue.empty()) {
        m_currentGoal = std::move(m_goalQueue.front());
        m_goalQueue.erase(m_goalQueue.begin());
        m_currentGoal->activate(this);
    }
    
    // Select new action if idle
    if (!m_currentGoal && m_actionCooldown <= 0 && m_self->canAct()) {
        selectAction();
        m_actionCooldown = 0.5f;
    }
}

void HolySwordWolfAI::selectAction() {
    float targetDist = getDistanceToTarget();
    float targetHP = getTargetHPRate();
    float selfHP = getSelfHPRate();
    
    // Action percentages based on distance and state
    int act01Per = 0, act02Per = 0, act03Per = 0, act04Per = 0;
    int act05Per = 0, act06Per = 0, act07Per = 0, act08Per = 0;
    int act09Per = 0, act10Per = 0, act11Per = 0, act12Per = 0, act13Per = 0;
    
    // Enhanced state behavior (similar to special effect 5401)
    if (m_isEnhanced) {
        // 2.6
        if (targetDist <= ATTACK_CLOSE) {
            if (isTargetBehind() && isTargetOnSide(true)) {
                act10Per = 100; // Enhanced spin right
            } else if (isTargetBehind() && isTargetOnSide(false)) {
                act11Per = 100; // Enhanced spin left
            } else {
                act08Per = 35;  // Enhanced combo
                act09Per = 65;  // Enhanced heavy
            }
        } else {
            act08Per = 65;
            act09Per = 35;
        }
    } else {
        // Normal state behavior
        if (targetDist > ATTACK_FAR) { // > 13.5
            act01Per = 10;  // Light combo
            act02Per = 60;  // Dash attack
            act03Per = 10;  // Spin attack
            act13Per = 20;  // Projectile
        } else if (targetDist > (ATTACK_FAR + ATTACK_MID) / 2) {
            act01Per = 20;
            act03Per = 25;
            act04Per = 5;   // Uppercut
            act13Per = 50;
        } else if (targetDist > ATTACK_MID) { // > 5.2
            act01Per = 40;
            act03Per = 35;
            act04Per = 20;
            act07Per = 5;   // Backstep
        } else if (targetDist > ATTACK_CLOSE) {
            // Check for backstep counters
            if (isTargetBehind() && getRandomInt(1, 100) <= 80) {
                if (isTargetOnSide(true)) {
                    act05Per = 100; // Backstep slash right
                } else {
                    act06Per = 100; // Backstep slash left
                }
            } else {
                act01Per = 35;
                act03Per = 35;
                act04Per = 5;
                act07Per = 25;
            }
        } else { // Very close
            act01Per = 15;
            act03Per = 15;
            act04Per = 30;
            act07Per = 10;
            act12Per = 30;  // Ground slam
        }
    }
    
    // Aggression adjustment based on player HP
    float aggressionMultiplier = (targetHP <= 0.3f) ? 1.0f : 
                                 (m_aggressionLevel < 20) ? 3.0f : 
                                 (m_aggressionLevel < 30) ? 4.0f : 5.0f;
    
    // Select action based on weighted random
    int totalWeight = act01Per + act02Per + act03Per + act04Per + act05Per + 
                     act06Per + act07Per + act08Per + act09Per + act10Per + 
                     act11Per + act12Per + act13Per;
    
    if (totalWeight == 0) return;
    
    int roll = getRandomInt(1, totalWeight);
    int cumulative = 0;
    
    // Execute selected action
    if (roll <= (cumulative += act01Per)) {
        // Action 1: Light combo
        addGoal(std::make_unique<MoveToTargetGoal>(ATTACK_MID, targetDist > 10));
        int comboRoll = getRandomInt(1, 100);
        if (comboRoll <= 10) {
            addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1));
        } else if (comboRoll <= 40) {
            addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1));
            addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_2));
        } else {
            addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1));
            addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_2));
            addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_3));
        }
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act02Per)) {
        // Action 2: Dash attack
        addGoal(std::make_unique<MoveToTargetGoal>(ATTACK_FAR));
        if (getRandomInt(1, 100) <= 30) {
            addGoal(std::make_unique<AttackGoal>(AttackType::DASH_ATTACK));
        } else {
            addGoal(std::make_unique<AttackGoal>(AttackType::DASH_ATTACK));
            addGoal(std::make_unique<AttackGoal>(AttackType::DASH_FOLLOWUP));
        }
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act03Per)) {
        // Action 3: Spin attack
        addGoal(std::make_unique<MoveToTargetGoal>(4.1f));
        addGoal(std::make_unique<AttackGoal>(AttackType::SPIN_ATTACK));
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act04Per)) {
        // Action 4: Uppercut
        addGoal(std::make_unique<MoveToTargetGoal>(1.5f));
        addGoal(std::make_unique<AttackGoal>(AttackType::UPPERCUT));
        m_aggressionLevel = std::max(0, m_aggressionLevel - 5);
    } else if (roll <= (cumulative += act05Per)) {
        // Action 5: Backstep slash right
        addGoal(std::make_unique<AttackGoal>(AttackType::BACKSTEP_SLASH_R));
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act06Per)) {
        // Action 6: Backstep slash left
        addGoal(std::make_unique<AttackGoal>(AttackType::BACKSTEP_SLASH_L));
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act07Per)) {
        // Action 7: Backstep
        addGoal(std::make_unique<StepGoal>(StepType::BACKSTEP));
        m_aggressionLevel = std::max(0, m_aggressionLevel - 5);
    } else if (roll <= (cumulative += act08Per)) {
        // Action 8: Enhanced combo (if enhanced)
        if (m_isEnhanced) {
            addGoal(std::make_unique<MoveToTargetGoal>(4.2f));
            if (getRandomInt(1, 100) <= 45) {
                addGoal(std::make_unique<AttackGoal>(AttackType::ENHANCED_COMBO_1));
            } else {
                addGoal(std::make_unique<AttackGoal>(AttackType::ENHANCED_COMBO_1));
                addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_2));
            }
        }
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act09Per)) {
        // Action 9: Enhanced heavy (if enhanced)
        if (m_isEnhanced) {
            addGoal(std::make_unique<MoveToTargetGoal>(3.5f));
            addGoal(std::make_unique<AttackGoal>(AttackType::ENHANCED_COMBO_2));
        }
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act10Per)) {
        // Action 10: Enhanced spin right
        addGoal(std::make_unique<AttackGoal>(AttackType::ENHANCED_SPIN_R));
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act11Per)) {
        // Action 11: Enhanced spin left
        addGoal(std::make_unique<AttackGoal>(AttackType::ENHANCED_SPIN_L));
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act12Per)) {
        // Action 12: Ground slam
        addGoal(std::make_unique<MoveToTargetGoal>(2.0f));
        if (getRandomInt(1, 100) <= 30) {
            addGoal(std::make_unique<AttackGoal>(AttackType::GROUND_SLAM));
        } else {
            addGoal(std::make_unique<AttackGoal>(AttackType::GROUND_SLAM));
            addGoal(std::make_unique<AttackGoal>(AttackType::GROUND_SLAM_FOLLOWUP));
        }
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act13Per)) {
        // Action 13: Projectile
        addGoal(std::make_unique<MoveToTargetGoal>(12.0f));
        addGoal(std::make_unique<AttackGoal>(AttackType::PROJECTILE));
        m_aggressionLevel += 10;
    }
    
    // Add after-action behavior
    int afterRoll = getRandomInt(1, 100);
    if (selfHP <= 0.1f && afterRoll > 60) {
        // Low HP defensive behavior
        if (afterRoll <= 80) {
            addGoal(std::make_unique<MoveToTargetGoal>(3.6f));
        } else {
            addGoal(std::make_unique<SidewayMoveGoal>(getRandomInt(0, 1), 2.5f));
        }
    } else if (afterRoll > 30) {
        // Normal after-action behavior
        if (afterRoll <= 40) {
            addGoal(std::make_unique<MoveToTargetGoal>(5.0f));
        } else if (afterRoll <= 80) {
            addGoal(std::make_unique<StepGoal>(StepType::BACKSTEP));
        } else {
            StepType sideStep = (getRandomInt(1, 100) <= 50) ? 
                                StepType::SIDESTEP_LEFT : StepType::SIDESTEP_RIGHT;
            addGoal(std::make_unique<StepGoal>(sideStep));
        }
    }
}

void HolySwordWolfAI::onDamaged(float damage, const Vector2D& sourcePos) {
    m_lastDamageTime = SDL_GetTicks() / 1000.0f;
    
    if (!m_isEnhanced && getDistanceToTarget() < 6.0f) {
        // Interrupt and counterattack
        clearGoals();
        
        float targetDist = getDistanceToTarget();

        int roll = getRandomInt(1, 100);
        if (targetDist <= 2.0f) {
            addGoal(std::make_unique<StepGoal>(StepType::BACKSTEP));
        } else if (targetDist <= 6.0f && roll <= 50) {
            addGoal(std::make_unique<StepGoal>(StepType::BACKSTEP));
        } else if (roll <= 75) {
            addGoal(std::make_unique<StepGoal>(StepType::SIDESTEP_LEFT));
        } else {
            addGoal(std::make_unique<StepGoal>(StepType::SIDESTEP_RIGHT));
        }
    }
}

void HolySwordWolfAI::onGuardBroken() {
    m_isGuardBroken = true;
    
    if (getDistanceToTarget() < 5.8f && getRandomInt(1, 100) <= 80) {
        clearGoals();
        addGoal(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1));
    }
}

void HolySwordWolfAI::onProjectileDetected(const Vector2D& projectilePos) {
    if (!m_isEnhanced) {
        float dist = getDistanceToTarget();
        
        if ((dist <= 8 && getRandomInt(1, 100) <= 0) ||    // Near
            (dist <= 25 && getRandomInt(1, 100) <= 30)) {   // Far
            clearGoals();
            if (getRandomInt(1, 100) <= 50) {
                addGoal(std::make_unique<StepGoal>(StepType::SIDESTEP_LEFT));
            } else {
                addGoal(std::make_unique<StepGoal>(StepType::SIDESTEP_RIGHT));
            }
        }
    }
}

// Utility function implementations
void HolySwordWolfAI::executeGoal(std::unique_ptr<AIGoal> goal) {
    if (m_currentGoal) {
        m_goalQueue.push_back(std::move(goal));
    } else {
        m_currentGoal = std::move(goal);
        m_currentGoal->activate(this);
    }
}

void HolySwordWolfAI::clearGoals() {
    if (m_currentGoal) {
        m_currentGoal->terminate(this);
        m_currentGoal.reset();
    }
    m_goalQueue.clear();
}

void HolySwordWolfAI::addGoal(std::unique_ptr<AIGoal> goal) {
    m_goalQueue.push_back(std::move(goal));
}

int HolySwordWolfAI::getRandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}

float HolySwordWolfAI::getRandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_rng);
}

// Utility function updates
float HolySwordWolfAI::getDistanceToTarget() const {
    std::cout << "Distance: " << m_self->getPosition().distance(m_target->getPosition()) << std::endl;
    return m_self->getPosition().distance(m_target->getPosition());
}

float HolySwordWolfAI::getAngleToTarget() const {
    Vector2D toTarget = m_target->getPosition() - m_self->getPosition();
    return atan2(toTarget.y, toTarget.x);
}

bool HolySwordWolfAI::isTargetBehind() const {
    float angle = getAngleToTarget();
    return std::abs(angle) > 2.44f; // ~140 degrees
}

bool HolySwordWolfAI::isTargetOnSide(bool checkRight) const {
    float angle = getAngleToTarget();
    if (checkRight) {
        return angle > 0 && angle < M_PI;
    } else {
        return angle < 0 && angle > -M_PI;
    }
}

float HolySwordWolfAI::getTargetHPRate() const {
    return m_target->getHealthPercentage();
}

float HolySwordWolfAI::getSelfHPRate() const {
    return m_self->getHealthPercentage();
}
