#include "Renderer.h"
#include "Player.h"
#include "Boss.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Renderer::Renderer(SDL_Renderer* renderer, int width, int height)
    : m_renderer(renderer), m_screenWidth(width), m_screenHeight(height), m_debugMode(false) {}

void Renderer::clear() {
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 30, 255);
    SDL_RenderClear(m_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::drawHealthBar(float x, float y, float width, float height, 
                           float percentage, SDL_Color color) {
    // Background
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, 255);
    SDL_Rect bgRect = {(int)x, (int)y, (int)width, (int)height};
    SDL_RenderFillRect(m_renderer, &bgRect);
    
    // Health
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_Rect healthRect = {(int)x, (int)y, (int)(width * percentage), (int)height};
    SDL_RenderFillRect(m_renderer, &healthRect);
    
    // Border
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(m_renderer, &bgRect);
}

void Renderer::drawStaminaBar(float x, float y, float width, float height, 
                            float percentage) {
    SDL_Color staminaColor = {255, 255, 0, 255};
    drawHealthBar(x, y, width, height, percentage, staminaColor);
}

void Renderer::drawEntity(const SDL_Rect& rect, SDL_Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_renderer, &rect);
}

void Renderer::drawUI(const Player* player, const Boss* boss) {
    // Player health bar
    drawHealthBar(20, m_screenHeight - 50, 200, 20, 
                 player->getHealthPercentage(), {0, 255, 0, 255});
    
    // Player stamina bar
    drawStaminaBar(20, m_screenHeight - 25, 200, 15, 
                  player->getStaminaPercentage());
    
    // Boss health bar
    drawHealthBar((float)m_screenWidth/2 - 150, 20, 300, 30, 
                 boss->getHealthPercentage(), {255, 0, 0, 255});
    
    // Debug mode indicator
    if (m_debugMode) {
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
        SDL_Rect debugRect = {10, 70, 150, 20};
        SDL_RenderDrawRect(m_renderer, &debugRect);
    }
}

void Renderer::drawCircle(int centerX, int centerY, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    int x = radius - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int err = tx - (radius << 1);

    while (x >= y) {
        SDL_RenderDrawPoint(m_renderer, centerX + x, centerY - y);
        SDL_RenderDrawPoint(m_renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(m_renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(m_renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(m_renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(m_renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(m_renderer, centerX - y, centerY - x);
        SDL_RenderDrawPoint(m_renderer, centerX - y, centerY + x);

        if (err <= 0) {
            y++;
            err += ty;
            ty += 2;
        }

        if (err > 0) {
            x--;
            tx += 2;
            err += tx - (radius << 1);
        }
    }
}

void Renderer::drawDebugInfo(const Player* player, const Boss* boss) {
    if (!m_debugMode) return;

    // Draw collision boxes (already in pixels)
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 255);  // Yellow for collision boxes
    SDL_Rect playerBox = player->getCollisionBox();
    SDL_Rect bossBox = boss->getCollisionBox();
    SDL_RenderDrawRect(m_renderer, &playerBox);
    SDL_RenderDrawRect(m_renderer, &bossBox);

    // Draw player attack range (convert meters to pixels)
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 100);  // Yellow for player attack range
    Vector2D playerPosPixels = GameUnits::toPixels(player->getPosition());
    int playerX = static_cast<int>(playerPosPixels.x);
    int playerY = static_cast<int>(playerPosPixels.y);
    int playerAttackRangePixels = static_cast<int>(GameUnits::toPixels(player->getAttackRange()));
    drawCircle(playerX, playerY, playerAttackRangePixels, {255, 255, 0, 100});

    // Draw boss attack range (convert meters to pixels)
    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 100);  // Red for boss attack range
    Vector2D bossPosPixels = GameUnits::toPixels(boss->getPosition());
    int bossX = static_cast<int>(bossPosPixels.x);
    int bossY = static_cast<int>(bossPosPixels.y);
    int bossAttackRangePixels = static_cast<int>(GameUnits::toPixels(boss->getAttackRange()));
    drawCircle(bossX, bossY, bossAttackRangePixels, {255, 0, 0, 100});

    // Draw boss attack hitbox when attacking (convert meters to pixels)
    if (boss->isAttacking()) {
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 255, 255);  // Magenta for active attack
        Circle attackCircle = boss->getAttackCircle();
        Vector2D attackCenterPixels = GameUnits::toPixels(Vector2D(attackCircle.x, attackCircle.y));
        int attackX = static_cast<int>(attackCenterPixels.x);
        int attackY = static_cast<int>(attackCenterPixels.y);
        int attackRadiusPixels = static_cast<int>(GameUnits::toPixels(attackCircle.r));
        drawCircle(attackX, attackY, attackRadiusPixels, {255, 0, 255, 255});
        
        SDL_SetRenderDrawColor(m_renderer, 200, 200, 255, 255);  // Light blue for sword
        SDL_Rect swordBox = boss->getSwordHitbox();
        SDL_RenderDrawRect(m_renderer, &swordBox);
    }

    // Draw boss state indicator (convert position to pixels, keep offsets in pixels)
    BossAnimState animState = boss->getAnimState();
    SDL_Color stateColor;
    switch (animState) {
        case BossAnimState::IDLE: stateColor = {0, 255, 0, 255}; break;        // Green
        case BossAnimState::MOVING: stateColor = {0, 0, 255, 255}; break;      // Blue
        case BossAnimState::ATTACKING: stateColor = {255, 0, 0, 255}; break;   // Red
        case BossAnimState::RECOVERING: stateColor = {255, 255, 0, 255}; break;// Yellow
        case BossAnimState::DAMAGED: stateColor = {128, 0, 128, 255}; break;   // Purple
        case BossAnimState::DYING: stateColor = {0, 0, 0, 255}; break;         // Black
        default: stateColor = {255, 255, 255, 255}; break;                     // White
    }
    drawCircle(bossX, bossY - 40, 5, stateColor);

    // Draw attack type indicator when attacking (convert position to pixels, keep offsets in pixels)
    if (animState == BossAnimState::ATTACKING) {
        BossAttackAnim attackAnim = boss->getCurrentAttackAnim();
        SDL_Color attackColor;
        switch (attackAnim) {
            case BossAttackAnim::HORIZONTAL_SWING: attackColor = {173, 216, 230, 255}; break; // Light Blue
            case BossAttackAnim::SPIN_ATTACK: attackColor = {0, 255, 255, 255}; break;       // Cyan
            case BossAttackAnim::OVERHEAD_SWING: attackColor = {255, 0, 255, 255}; break;    // Magenta
            case BossAttackAnim::UPPERCUT: attackColor = {255, 165, 0, 255}; break;          // Orange
            case BossAttackAnim::GROUND_SLAM: attackColor = {165, 42, 42, 255}; break;       // Brown
            case BossAttackAnim::DASH_ATTACK: attackColor = {255, 192, 203, 255}; break;     // Pink
            case BossAttackAnim::PROJECTILE: attackColor = {0, 255, 0, 255}; break;          // Lime
            case BossAttackAnim::BACKSTEP_SLASH: attackColor = {0, 128, 128, 255}; break;    // Teal
            default: attackColor = {255, 255, 255, 255}; break;                              // White
        }
        drawCircle(bossX + 10, bossY - 40, 5, attackColor);
    }

    // Draw player attack hitbox if attacking (already in pixels)
    if (player->isAttacking()) {
        SDL_SetRenderDrawColor(m_renderer, 0, 255, 255, 255);  // Cyan
        SDL_Rect playerSwordBox = player->getSwordHitbox();
        SDL_RenderDrawRect(m_renderer, &playerSwordBox);
    }
}
