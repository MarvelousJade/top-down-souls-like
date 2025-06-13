#include "Sif.h"
#include "Vector2D.h"
#include <random>
#include <iostream>
#include <iomanip>

HolySwordWolfAI* g_sifAI = nullptr;

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

void AttackGoal::terminate(HolySwordWolfAI* ai) {
    // Cancel attack if boss has the method
    if (ai->m_self->isAttacking() || ai->m_self->isRecovering()) {
        // Uncomment when cancelAttack is added to Boss:
        ai->m_self->cancelAttack();
        
        if (ai->isDebugEnabled()) {
            std::cout << "[AI] Attack terminated" << std::endl;
        }
    }
    
    currentTime = 0;
}

void MoveToTargetGoal::activate(HolySwordWolfAI* ai) {
    // Calculate target position based on desired distance
    Vector2D toTarget = ai->m_target->getPosition() - ai->m_self->getPosition();
    float currentDist = toTarget.length();
    
    // Add some variance to target distance to make movement more natural
    float variance = ai->getRandomFloat(-0.5f, 0.5f);
    float adjustedTargetDist = targetDistance + variance;
    
    if (std::abs(currentDist - adjustedTargetDist) > 1.0f) {  // Increased threshold
        Vector2D targetPos;
        if (currentDist > adjustedTargetDist) {
            // Move closer
            targetPos = ai->m_self->getPosition() + toTarget.normalized() * (currentDist - adjustedTargetDist);
        } else {
            // Move away
            targetPos = ai->m_self->getPosition() - toTarget.normalized() * (adjustedTargetDist - currentDist);
        }
        
        // Add slight perpendicular offset for more natural movement
        Vector2D perpendicular(-toTarget.y, toTarget.x);
        perpendicular = perpendicular.normalized();
        targetPos = targetPos + perpendicular * ai->getRandomFloat(-1.0f, 1.0f);
        
        float speedMultiplier = walk ? 0.5f : 1.0f;
        ai->m_self->startMoving(targetPos, speedMultiplier);
    }
}

bool MoveToTargetGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    float currentDist = ai->getDistanceToTarget();
    
    // More lenient completion check
    if (std::abs(currentDist - targetDistance) < 1.5f || !ai->m_self->isMoving()) {
        ai->m_self->stopMoving();
        return true;
    }
    
    // Only recalculate if target moved significantly
    static Vector2D lastTargetPos = ai->m_target->getPosition();
    if (lastTargetPos.distance(ai->m_target->getPosition()) > 2.0f) {
        lastTargetPos = ai->m_target->getPosition();
        activate(ai); // Recalculate path
    }
    
    return false;
}

void MoveToTargetGoal::terminate(HolySwordWolfAI* ai) {
    // Stop any ongoing movement
    ai->m_self->stopMoving();
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
    
    ai->m_self->performStep(stepDir, stepDistance); // Scale up distance
}

bool StepGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    currentProgress += deltaTime;
    return currentProgress >= 0.5f; // Quick step
}

void StepGoal::terminate(HolySwordWolfAI* ai) {
    // Reset progress
    currentProgress = 0;
}

void SidewayMoveGoal::activate(HolySwordWolfAI* ai) {
    currentTime = 0;
    
    Vector2D toTarget = (ai->m_target->getPosition() - ai->m_self->getPosition()).normalized();
    Vector2D sideDir = moveRight ? Vector2D(toTarget.y, -toTarget.x) : Vector2D(-toTarget.y, toTarget.x);
    
    // Vector2D targetPos = ai->m_self->getPosition() + sideDir * 100.0f;
    // Move in an arc rather than straight sideways
    Vector2D targetPos = ai->m_self->getPosition() + sideDir * 5.0f + toTarget * 2.0f;
    ai->m_self->startMoving(targetPos, 0.7f);
}

bool SidewayMoveGoal::update(HolySwordWolfAI* ai, float deltaTime) {
    currentTime += deltaTime;
    if (currentTime >= duration || !ai->m_self->isMoving()) {
        ai->m_self->stopMoving();
        return true;
    }
    return false;
}

void SidewayMoveGoal::terminate(HolySwordWolfAI* ai) {
    // Stop the sideways movement
    ai->m_self->stopMoving();
    
    // Reset timer
    currentTime = 0;
}

HolySwordWolfAI::HolySwordWolfAI(Boss* entity, Player* player)
    : m_self(entity), m_target(player), m_rng(std::random_device{}()),
      m_isEnhanced(false), m_enhancedTimer(0), m_lastDamageTime(0),
      m_isGuardBroken(false), m_actionCooldown(0), m_aggressionLevel(0) {
    g_sifAI = this;  // Set global pointer for debug access
    m_debugEnabled = true;  // Enable debug by default
}

// Debug helper function implementations
std::string HolySwordWolfAI::goalTypeToString(GoalType type) const {
    switch (type) {
        case GoalType::ATTACK: return "ATTACK";
        case GoalType::MOVE_TO_TARGET: return "MOVE";
        case GoalType::STEP: return "STEP";
        case GoalType::SIDEWAY_MOVE: return "SIDEWAY";
        default: return "UNKNOWN";
    }
}

std::string HolySwordWolfAI::attackTypeToString(AttackType type) const {
    switch (type) {
        case AttackType::LIGHT_COMBO_1: return "Light1";
        case AttackType::LIGHT_COMBO_2: return "Light2";
        case AttackType::LIGHT_COMBO_3: return "Light3";
        case AttackType::DASH_ATTACK: return "Dash";
        case AttackType::DASH_FOLLOWUP: return "DashFollow";
        case AttackType::SPIN_ATTACK: return "Spin";
        case AttackType::UPPERCUT: return "Uppercut";
        case AttackType::BACKSTEP_SLASH_R: return "BackslashR";
        case AttackType::BACKSTEP_SLASH_L: return "BackslashL";
        case AttackType::GROUND_SLAM: return "Slam";
        case AttackType::GROUND_SLAM_FOLLOWUP: return "SlamFollow";
        case AttackType::PROJECTILE: return "Projectile";
        case AttackType::ENHANCED_COMBO_1: return "EnhCombo1";
        case AttackType::ENHANCED_COMBO_2: return "EnhCombo2";
        case AttackType::ENHANCED_SPIN_R: return "EnhSpinR";
        case AttackType::ENHANCED_SPIN_L: return "EnhSpinL";
        default: return "Unknown";
    }
}

std::string HolySwordWolfAI::stepTypeToString(StepType type) const {
    switch (type) {
        case StepType::BACKSTEP: return "Backstep";
        case StepType::SIDESTEP_LEFT: return "SideLeft";
        case StepType::SIDESTEP_RIGHT: return "SideRight";
        default: return "Unknown";
    }
}

void HolySwordWolfAI::logGoalAddition(const std::string& goalName, const std::string& reason) {
    if (!m_debugEnabled) return;
    
    GoalDebugInfo info;
    info.goalName = goalName;
    info.reason = reason;
    info.timestamp = SDL_GetTicks() / 1000.0f;
    
    m_goalHistory.push_back(info);
    if (m_goalHistory.size() > MAX_HISTORY_SIZE) {
        m_goalHistory.pop_front();
    }
    
    // Console logging
    std::cout << "[AI " << std::fixed << std::setprecision(2) << info.timestamp 
              << "] Adding Goal: " << goalName << " | Reason: " << reason << std::endl;
}

void HolySwordWolfAI::updateGoalQueueDebug() {
    if (!m_debugEnabled) return;
    
    m_goalQueueDebug.clear();
    
    // Add current goal
    if (m_currentGoal) {
        std::string current = "CURRENT: ";
        GoalType type = m_currentGoal->getType();
        current += goalTypeToString(type);
        
        // Add specific details
        if (type == GoalType::ATTACK) {
            AttackGoal* attack = static_cast<AttackGoal*>(m_currentGoal.get());
            current += " (" + attackTypeToString(attack->getAttackType()) + ")";
        } else if (type == GoalType::MOVE_TO_TARGET) {
            MoveToTargetGoal* move = static_cast<MoveToTargetGoal*>(m_currentGoal.get());
            current += " (dist: " + std::to_string(move->getTargetDistance()) + ")";
        } else if (type == GoalType::STEP) {
            StepGoal* step = static_cast<StepGoal*>(m_currentGoal.get());
            current += " (" + stepTypeToString(step->getStepType()) + ")";
        } else if (type == GoalType::SIDEWAY_MOVE) {
            SidewayMoveGoal* side = static_cast<SidewayMoveGoal*>(m_currentGoal.get());
            current += side->isMoveRight() ? " (Right)" : " (Left)";
        }
        
        m_currentGoalDebug = current;
    } else {
        m_currentGoalDebug = "CURRENT: None";
    }
    
    // Add queued goals
    for (size_t i = 0; i < m_goalQueue.size(); ++i) {
        std::string queued = "QUEUE[" + std::to_string(i) + "]: ";
        GoalType type = m_goalQueue[i]->getType();
        queued += goalTypeToString(type);
        
        // Add specific details
        if (type == GoalType::ATTACK) {
            AttackGoal* attack = static_cast<AttackGoal*>(m_goalQueue[i].get());
            queued += " (" + attackTypeToString(attack->getAttackType()) + ")";
        } else if (type == GoalType::MOVE_TO_TARGET) {
            MoveToTargetGoal* move = static_cast<MoveToTargetGoal*>(m_goalQueue[i].get());
            queued += " (dist: " + std::to_string(move->getTargetDistance()) + ")";
        } else if (type == GoalType::STEP) {
            StepGoal* step = static_cast<StepGoal*>(m_goalQueue[i].get());
            queued += " (" + stepTypeToString(step->getStepType()) + ")";
        }
        
        m_goalQueueDebug.push_back(queued);
    }
}

// Main AI Update
void HolySwordWolfAI::update(float deltaTime) {
    // Update debug timer
    if (m_debugEnabled) {
        m_debugTimer += deltaTime;
    }
    
    float currentTime = SDL_GetTicks() / 1000.0f;

    // Update boss facing direction (smoother rotation)
    if (m_self->canAct() && !m_self->isMoving()) {
        Vector2D toTarget = m_target->getPosition() - m_self->getPosition();
        Vector2D currentFacing = m_self->getSwordBase() - m_self->getPosition();
        
        // Lerp facing direction for smoother rotation
        float lerpSpeed = 5.0f * deltaTime;
        Vector2D newFacing = currentFacing.normalized() + (toTarget.normalized() - currentFacing.normalized()) * lerpSpeed;
        m_self->setFacingDirection(newFacing.normalized());
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
            if (m_debugEnabled) {
                std::cout << "[AI] Enhanced state ended" << std::endl;
            }
        }
    }
    
    // Process current goal
    if (m_currentGoal) {
        m_idleTimer = 0; // Reset idle timer when executing goals
        
        if (m_currentGoal->update(this, deltaTime)) {
            if (m_debugEnabled) {
                std::cout << "[AI] Goal completed: " << goalTypeToString(m_currentGoal->getType()) << std::endl;
            }

            // Track attack completion
            if (m_currentGoal->getType() == GoalType::ATTACK) {
                m_lastAttackTime = currentTime;
            }

            m_currentGoal->terminate(this);
            m_currentGoal.reset();
        }
    }
    
    // Get next goal from queue
    if (!m_currentGoal && !m_goalQueue.empty()) {
        m_currentGoal = std::move(m_goalQueue.front());
        m_goalQueue.erase(m_goalQueue.begin());
        m_currentGoal->activate(this);
        if (m_debugEnabled) {
            std::cout << "[AI] Activating next goal: " << goalTypeToString(m_currentGoal->getType()) << std::endl;
        }
    }
    
    // Update debug info
    updateGoalQueueDebug();
    
    // Select new action if idle - with variable cooldown
    float dynamicCooldown = m_actionCooldown;
    if (getDistanceToTarget() > ATTACK_FAR) {
        dynamicCooldown = 0.3f; // Faster decisions when far
    } else if (currentTime - m_lastAttackTime < 1.0f) {
        dynamicCooldown = 1.0f; // Longer cooldown after recent attack
    }
    
    if (!m_currentGoal && m_actionCooldown <= 0 && m_self->canAct()) {
        // Add idle behavior if standing still too long
        if (m_idleTimer > 2.0f && getRandomInt(1, 100) <= 30) {
            selectIdleBehavior();
        } else {
            selectAction();
        }
        
        m_actionCooldown = dynamicCooldown;
    }
}

void HolySwordWolfAI::selectIdleBehavior() {
    if (m_debugEnabled) {
        std::cout << "[AI] Selecting idle behavior" << std::endl;
    }
    
    float targetDist = getDistanceToTarget();
    int roll = getRandomInt(1, 100);
    
    if (targetDist > ATTACK_FAR) {
        // Approach slowly when far
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(ATTACK_MID, true), "Idle: Walk closer");
    } else if (roll <= 40) {
        // Circle around target
        bool circleRight = getRandomInt(1, 100) <= 50;
        addGoalWithReason(std::make_unique<SidewayMoveGoal>(circleRight, 1.5f), "Idle: Circle");
    } else if (roll <= 70) {
        // Adjust distance slightly
        float newDist = targetDist + getRandomFloat(-2.0f, 2.0f);
        newDist = std::max(ATTACK_CLOSE - 1.0f, std::min(ATTACK_MID + 1.0f, newDist));
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(newDist, true), "Idle: Adjust position");
    }
    
    m_idleTimer = 0;
}

void HolySwordWolfAI::selectAction() {
    float targetDist = getDistanceToTarget();
    float targetHP = getTargetHPRate();
    float selfHP = getSelfHPRate();
    float timeSinceLastAttack = (SDL_GetTicks() / 1000.0f) - m_lastAttackTime;
    
    // Action percentages based on distance and state
    // 1-light combo, 2-dash attack, 3-spin attack, 4-Uppercut, 
    // 5-backstep slash right, 6-backstep slash left, 7-backstep,
    // 8-11 enhanced, 12-ground slam, 14-sideway move, 15-walk to target
    int act01Per = 0, act02Per = 0, act03Per = 0, act04Per = 0;
    int act05Per = 0, act06Per = 0, act07Per = 0, act08Per = 0;
    int act09Per = 0, act10Per = 0, act11Per = 0, act12Per = 0, act13Per = 0, act14Per = 0;
    int act15Per = 0; 
    std::stringstream reasonBuilder;
    reasonBuilder << "Dist:" << std::fixed << std::setprecision(1) << targetDist;
    
    // Enhanced state behavior (similar to special effect 5401)
    if (m_isEnhanced) {
        reasonBuilder << " Enhanced";
        if (targetDist <= ATTACK_CLOSE) { // <=6
            if (isTargetBehind() && isTargetOnSide(true)) {
                act10Per = 100; // Enhanced spin right
                reasonBuilder << " TargetBehindRight";
            } else if (isTargetBehind() && isTargetOnSide(false)) {
                act11Per = 100; // Enhanced spin left
                reasonBuilder << " TargetBehindLeft";
            } else {
                act08Per = 35;  // Enhanced combo
                act09Per = 65;  // Enhanced heavy
                reasonBuilder << " CloseRange";
            }
        } else {
            act08Per = 65;
            act09Per = 35;
            reasonBuilder << " MidRange";
        }
    } else {
        // Normal state behavior
        if (targetDist > ATTACK_FAR) { // > 12
            act02Per = 50;  // Dash attack
            act14Per = 20;  // Sideway move
            act15Per = 30;  // Walk to position
            reasonBuilder << " FarRange";
        } else if (targetDist > (ATTACK_MID) / 2) { // > 8
            act02Per = 50;  // Dash attack
            act14Per = 30;
            act15Per = 20;
            reasonBuilder << " MidFar";
        } else if (targetDist > ATTACK_CLOSE) { // > 6
            act01Per = 70;
            act02Per = 5;  // Dash attack
            act07Per = 5;   // Backstep
            act15Per = 20; 
            reasonBuilder << " MidClose";
        } else if (targetDist > ATTACK_VERY_CLOSE) { // > 4
                act01Per = 35;
                act03Per = 35;
                act04Per = 5;
                act07Per = 25;
                reasonBuilder << " Close";
        } else { // Very close
            // Check for backstep counters
            if (isTargetBehind() && getRandomInt(1, 100) <= 60) {
                if (isTargetOnSide(true)) {
                    act05Per = 40; // Backstep slash right
                } else {
                    act06Per = 40; // Backstep slash left
                }
                reasonBuilder << " VeryCloseBehind";
            }
            act01Per = 10;
            act03Per = 10;
            act04Per = 20;
            act07Per = 20;
            reasonBuilder << " VeryClose";
        }

        // Reduce attack frequency if recently attacked
        if (timeSinceLastAttack < 1.5f) {
            act01Per = act01Per / 2;
            act02Per = act02Per / 2;
            act03Per = act03Per / 2;
            act14Per += 20;
            act15Per += 20;
            reasonBuilder << " RecentAttack";
        }
    }
    
    m_lastActionReason = reasonBuilder.str();
    
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
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(ATTACK_MID, targetDist > 10), m_lastActionReason + " LightCombo");
        int comboRoll = getRandomInt(1, 100);
        if (comboRoll <= 10) {
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1), "Combo1");
        } else if (comboRoll <= 40) {
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1), "Combo1-2");
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_2), "Combo2");
        } else {
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1), "Combo1-3");
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_2), "Combo2");
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_3), "Combo3");
        }
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act02Per)) {
        // Action 2: Dash attack
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(ATTACK_CLOSE), m_lastActionReason + " DashAttack");
        if (getRandomInt(1, 100) <= 30 && targetDist < ATTACK_CLOSE) {
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::DASH_ATTACK), "DashSingle");
        } else if (targetDist < ATTACK_CLOSE){
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::DASH_ATTACK), "DashCombo");
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::DASH_FOLLOWUP), "DashFollow");
        }
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act03Per)) {
        // Action 3: Spin attack
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(4.1f), m_lastActionReason + " SpinAttack");
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::SPIN_ATTACK), "Spin");
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act04Per)) {
        // Action 4: Uppercut
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(1.5f), m_lastActionReason + " Uppercut");
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::UPPERCUT), "Uppercut");
        m_aggressionLevel = std::max(0, m_aggressionLevel - 5);
    } else if (roll <= (cumulative += act05Per)) {
        // Action 5: Backstep slash right
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::BACKSTEP_SLASH_R), m_lastActionReason + " BackslashR");
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act06Per)) {
        // Action 6: Backstep slash left
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::BACKSTEP_SLASH_L), m_lastActionReason + " BackslashL");
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act07Per)) {
        // Action 7: Backstep
        addGoalWithReason(std::make_unique<StepGoal>(StepType::BACKSTEP), m_lastActionReason + " Backstep");
        m_aggressionLevel = std::max(0, m_aggressionLevel - 5);
    } else if (roll <= (cumulative += act08Per)) {
        // Action 8: Enhanced combo (if enhanced)
        if (m_isEnhanced) {
            addGoalWithReason(std::make_unique<MoveToTargetGoal>(4.2f), m_lastActionReason + " EnhCombo");
            if (getRandomInt(1, 100) <= 45) {
                addGoalWithReason(std::make_unique<AttackGoal>(AttackType::ENHANCED_COMBO_1), "EnhCombo1");
            } else {
                addGoalWithReason(std::make_unique<AttackGoal>(AttackType::ENHANCED_COMBO_1), "EnhCombo1-2");
                addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_2), "ComboFollow");
            }
        }
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act09Per)) {
        // Action 9: Enhanced heavy (if enhanced)
        if (m_isEnhanced) {
            addGoalWithReason(std::make_unique<MoveToTargetGoal>(3.5f), m_lastActionReason + " EnhHeavy");
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::ENHANCED_COMBO_2), "EnhHeavy");
        }
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act10Per)) {
        // Action 10: Enhanced spin right
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::ENHANCED_SPIN_R), m_lastActionReason + " EnhSpinR");
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act11Per)) {
        // Action 11: Enhanced spin left
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::ENHANCED_SPIN_L), m_lastActionReason + " EnhSpinL");
        m_aggressionLevel += 20;
    } else if (roll <= (cumulative += act12Per)) {
        // Action 12: Ground slam
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(2.0f), m_lastActionReason + " GroundSlam");
        if (getRandomInt(1, 100) <= 30) {
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::GROUND_SLAM), "SlamSingle");
        } else {
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::GROUND_SLAM), "SlamCombo");
            addGoalWithReason(std::make_unique<AttackGoal>(AttackType::GROUND_SLAM_FOLLOWUP), "SlamFollow");
        }
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act13Per)) {
        // Action 13: Projectile
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(5.0f), m_lastActionReason + " Projectile");
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::PROJECTILE), "Projectile");
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act14Per)) {
        // Action 14: SidewayMove
        addGoalWithReason(std::make_unique<SidewayMoveGoal>(true, 2.0f), m_lastActionReason + " SidewayMove");
        m_aggressionLevel += 10;
    } else if (roll <= (cumulative += act15Per)) {
        addGoalWithReason(std::make_unique<MoveToTargetGoal>(true, 5.0f), m_lastActionReason + " Walk to target");
    }

    
    // Add after-action behavior
    int afterRoll = getRandomInt(1, 100);
    if (selfHP <= 0.1f && afterRoll > 60) {
        // Low HP defensive behavior
        if (afterRoll <= 80) {
            addGoalWithReason(std::make_unique<MoveToTargetGoal>(3.6f), "LowHP Retreat");
        } else {
            addGoalWithReason(std::make_unique<SidewayMoveGoal>(getRandomInt(0, 1), 2.5f), "LowHP Sidestep");
        }
    } else if (afterRoll > 70) {
        // Normal after-action behavior
        if (afterRoll <= 80) {
            addGoalWithReason(std::make_unique<MoveToTargetGoal>(5.0f), "PostAttack Distance");
        } else if (afterRoll <= 90) {
            addGoalWithReason(std::make_unique<StepGoal>(StepType::BACKSTEP), "PostAttack Backstep");
        } else {
            StepType sideStep = (getRandomInt(1, 100) <= 50) ? 
                                StepType::SIDESTEP_LEFT : StepType::SIDESTEP_RIGHT;
            addGoalWithReason(std::make_unique<StepGoal>(sideStep), "PostAttack Sidestep");
        }
    }
    
    if (m_debugEnabled) {
        std::cout << "[AI] Action selection complete. Aggression: " << m_aggressionLevel << std::endl;
    }
}

void HolySwordWolfAI::onDamaged(float damage, const Vector2D& sourcePos) {
    m_lastDamageTime = SDL_GetTicks() / 1000.0f;
    
    if (m_debugEnabled) {
        std::cout << "[AI] Damaged for " << damage << " HP" << std::endl;
    }
    
    if (!m_isEnhanced && getDistanceToTarget() < 6.0f) {
        // Interrupt and counterattack
        clearGoals();
        
        float targetDist = getDistanceToTarget();

        int roll = getRandomInt(1, 100);
        if (targetDist <= 2.0f) {
            addGoalWithReason(std::make_unique<StepGoal>(StepType::BACKSTEP), "Damage Response: Too close");
        } else if (targetDist <= 6.0f && roll <= 50) {
            addGoalWithReason(std::make_unique<StepGoal>(StepType::BACKSTEP), "Damage Response: Close");
        } else if (roll <= 75) {
            addGoalWithReason(std::make_unique<StepGoal>(StepType::SIDESTEP_LEFT), "Damage Response: Dodge left");
        } else {
            addGoalWithReason(std::make_unique<StepGoal>(StepType::SIDESTEP_RIGHT), "Damage Response: Dodge right");
        }
    }
}

void HolySwordWolfAI::onGuardBroken() {
    m_isGuardBroken = true;
    
    if (m_debugEnabled) {
        std::cout << "[AI] Guard broken!" << std::endl;
    }
    
    if (getDistanceToTarget() < 5.8f && getRandomInt(1, 100) <= 80) {
        clearGoals();
        addGoalWithReason(std::make_unique<AttackGoal>(AttackType::LIGHT_COMBO_1), "Guard Break Punish");
    }
}

void HolySwordWolfAI::onProjectileDetected(const Vector2D& projectilePos) {
    if (m_debugEnabled) {
        std::cout << "[AI] Projectile detected" << std::endl;
    }
    
    if (!m_isEnhanced) {
        float dist = getDistanceToTarget();
        
        if ((dist <= 8 && getRandomInt(1, 100) <= 0) ||    // Near
            (dist <= 25 && getRandomInt(1, 100) <= 30)) {   // Far
            clearGoals();
            if (getRandomInt(1, 100) <= 50) {
                addGoalWithReason(std::make_unique<StepGoal>(StepType::SIDESTEP_LEFT), "Projectile Dodge Left");
            } else {
                addGoalWithReason(std::make_unique<StepGoal>(StepType::SIDESTEP_RIGHT), "Projectile Dodge Right");
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

// Updated clearGoals in HolySwordWolfAI to use forceIdle:
void HolySwordWolfAI::clearGoals() {
    if (m_debugEnabled) {
        std::cout << "[AI] Clearing all goals" << std::endl;
    }

    if (m_currentGoal) {
        m_currentGoal->terminate(this);
        m_currentGoal.reset();
    }
    
    // Force boss to idle state when clearing all goals
    // Uncomment when forceIdle is added:
    // m_self->forceIdle();
    
    // For now, just stop movement
    m_self->stopMoving();
    
    m_goalQueue.clear();
    m_actionCooldown = 0;
}

void HolySwordWolfAI::addGoal(std::unique_ptr<AIGoal> goal) {
    m_goalQueue.push_back(std::move(goal));
}

void HolySwordWolfAI::addGoalWithReason(std::unique_ptr<AIGoal> goal, const std::string& reason) {
    if (m_debugEnabled) {
        std::string goalName = goalTypeToString(goal->getType());
        
        // Add specific details
        if (goal->getType() == GoalType::ATTACK) {
            AttackGoal* attack = static_cast<AttackGoal*>(goal.get());
            goalName += " (" + attackTypeToString(attack->getAttackType()) + ")";
        } else if (goal->getType() == GoalType::MOVE_TO_TARGET) {
            MoveToTargetGoal* move = static_cast<MoveToTargetGoal*>(goal.get());
            goalName += " (dist: " + std::to_string(move->getTargetDistance()) + ")";
        } else if (goal->getType() == GoalType::STEP) {
            StepGoal* step = static_cast<StepGoal*>(goal.get());
            goalName += " (" + stepTypeToString(step->getStepType()) + ")";
        }
        
        logGoalAddition(goalName, reason);
    }
    
    addGoal(std::move(goal));
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
    float distance =  m_self->getPosition().distance(m_target->getPosition());
    std::cout << "Distance: " << distance << std::endl;
    return distance;
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
