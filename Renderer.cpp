#include "Renderer.h"
#include "Player.h"
#include "Boss.h"
#include "Sif.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// External AI pointer for debug access
extern HolySwordWolfAI* g_sifAI;

Renderer::Renderer(SDL_Renderer* renderer, int width, int height)
    : m_renderer(renderer), m_screenWidth(width), m_screenHeight(height), m_debugMode(false), m_font(nullptr), m_smallFont(nullptr) {

    if( TTF_Init() == -1 ) {
            printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
    }

    const char* fontPaths[] = {
        "assets/fonts/Montserrat/Montserrat-VariableFont_wght.ttf",  // Project fonts folder
        "fonts/arial.ttf",
        "arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",  // Linux
        "/System/Library/Fonts/Helvetica.ttc",  // macOS
        "C:/Windows/Fonts/arial.ttf",  // Windows
        nullptr
    };
    
    // Try to load font from various paths
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        m_font = TTF_OpenFont(fontPaths[i], 14);
        m_smallFont = TTF_OpenFont(fontPaths[i], 11);
        if (m_font && m_smallFont) {
            std::cout << "Font loaded from: " << fontPaths[i] << std::endl;
            break;
        } else {
            std::cerr << "Unable to open font from fontPaths. "<< TTF_GetError() << std::endl;
        }
    }
    
    if (!m_font || !m_smallFont) {
        std::cerr << "Failed to load font! Debug text will not be displayed properly." << std::endl;
    }
}

Renderer::~Renderer() {
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
    if (m_smallFont) {
        TTF_CloseFont(m_smallFont);
        m_smallFont = nullptr;
    }
}

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

void Renderer::drawTextFallback(const std::string& text, int x, int y, SDL_Color color) {
    // Enhanced fallback rendering with better character representation
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    int charWidth = 5;
    int charHeight = 8;
    int spacing = 1;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        int charX = x + i * (charWidth + spacing);
        
        if (c == ' ') continue;
        
        // Draw a simple representation based on character type
        if (c >= '0' && c <= '9') {
            // Numbers - draw as filled rectangle
            SDL_Rect charRect = {charX, y, charWidth, charHeight};
            SDL_RenderFillRect(m_renderer, &charRect);
        } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            // Letters - draw as outlined rectangle
            SDL_Rect charRect = {charX, y, charWidth, charHeight};
            SDL_RenderDrawRect(m_renderer, &charRect);
        } else if (c == ':' || c == '|' || c == '!' || c == 'I' || c == 'l') {
            // Vertical characters - draw as vertical line
            SDL_RenderDrawLine(m_renderer, charX + charWidth/2, y, charX + charWidth/2, y + charHeight);
        } else if (c == '-' || c == '_' || c == '=') {
            // Horizontal characters - draw as horizontal line
            SDL_RenderDrawLine(m_renderer, charX, y + charHeight/2, charX + charWidth, y + charHeight/2);
        } else if (c == '.' || c == ',') {
            // Small punctuation - draw as small rect
            SDL_Rect dotRect = {charX + charWidth/2 - 1, y + charHeight - 2, 2, 2};
            SDL_RenderFillRect(m_renderer, &dotRect);
        } else if (c == '[' || c == ']' || c == '(' || c == ')') {
            // Brackets - draw as partial rectangles
            if (c == '[' || c == '(') {
                SDL_RenderDrawLine(m_renderer, charX, y, charX, y + charHeight);
                SDL_RenderDrawLine(m_renderer, charX, y, charX + 2, y);
                SDL_RenderDrawLine(m_renderer, charX, y + charHeight, charX + 2, y + charHeight);
            } else {
                SDL_RenderDrawLine(m_renderer, charX + charWidth, y, charX + charWidth, y + charHeight);
                SDL_RenderDrawLine(m_renderer, charX + charWidth - 2, y, charX + charWidth, y);
                SDL_RenderDrawLine(m_renderer, charX + charWidth - 2, y + charHeight, charX + charWidth, y + charHeight);
            }
        } else {
            // Default - draw as small filled rectangle
            SDL_Rect charRect = {charX + 1, y + 1, charWidth - 2, charHeight - 2};
            SDL_RenderFillRect(m_renderer, &charRect);
        }
    }
}

void Renderer::drawText(const std::string& text, int x, int y, SDL_Color color) {
    if (!m_font) {
        drawTextFallback(text, x, y, color);
        return;
    }
    
    // Use appropriate font based on text length or position
    TTF_Font* fontToUse = (text.length() > 30 || y > m_screenHeight - 100) ? m_smallFont : m_font;
    if (!fontToUse) fontToUse = m_font;  // Fallback to main font if small font failed
    
    // Render text to surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    if (!textSurface) {
        // If TTF rendering fails, use fallback
        drawTextFallback(text, x, y, color);
        return;
    }
    
    // Create texture from surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        drawTextFallback(text, x, y, color);
        return;
    }
    
    // Get text dimensions
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    
    // Create destination rectangle
    SDL_Rect destRect = {x, y, textWidth, textHeight};
    
    // Render the text texture
    SDL_RenderCopy(m_renderer, textTexture, nullptr, &destRect);
    
    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Renderer::drawRect(int x, int y, int w, int h, SDL_Color color, bool filled) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    if (filled) {
        SDL_RenderFillRect(m_renderer, &rect);
    } else {
        SDL_RenderDrawRect(m_renderer, &rect);
    }
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

    // AI Debug display
    if (m_debugMode && m_showAIDebug) {
        drawAIDebugInfo();
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

void Renderer::drawAIDebugInfo() {
    if (!g_sifAI || !g_sifAI->isDebugEnabled()) return;
    
    // Background panel for AI debug info
    SDL_Color bgColor = {0, 0, 0, 200};
    drawRect(m_screenWidth - 350, 100, 340, 400, bgColor, true);
    
    // Title
    SDL_Color titleColor = {255, 255, 255, 255};
    drawText("===  AI DEBUG INFO  ===", m_screenWidth - 340, 110, titleColor);
    
    int yPos = 130;
    
    // AI State
    SDL_Color stateColor = {255, 255, 0, 255};
    std::stringstream ss;
    ss << "Enhanced: " << (g_sifAI->isEnhanced() ? "YES" : "NO");
    drawText(ss.str(), m_screenWidth - 340, yPos, stateColor);
    yPos += 15;
    
    ss.str("");
    ss << "Aggression: " << g_sifAI->getAggressionLevel();
    drawText(ss.str(), m_screenWidth - 340, yPos, stateColor);
    yPos += 15;
    
    ss.str("");
    ss << "Cooldown: " << std::fixed << std::setprecision(2) << g_sifAI->getActionCooldown();
    drawText(ss.str(), m_screenWidth - 340, yPos, stateColor);
    yPos += 20;
    
    // Current Goal
    SDL_Color currentColor = {0, 255, 0, 255};
    drawText("CURRENT GOAL:", m_screenWidth - 340, yPos, currentColor);
    yPos += 15;
    drawText(g_sifAI->getCurrentGoalDebug(), m_screenWidth - 330, yPos, currentColor);
    yPos += 20;
    
    // Goal Queue
    SDL_Color queueColor = {100, 200, 255, 255};
    drawText("GOAL QUEUE:", m_screenWidth - 340, yPos, queueColor);
    yPos += 15;
    
    const auto& goalQueue = g_sifAI->getGoalQueueDebug();
    if (goalQueue.empty()) {
        drawText("  [Empty]", m_screenWidth - 330, yPos, queueColor);
        yPos += 15;
    } else {
        for (const auto& goal : goalQueue) {
            drawText("  " + goal, m_screenWidth - 330, yPos, queueColor);
            yPos += 15;
            if (yPos > 350) break; // Don't overflow the panel
        }
    }
    
    yPos += 10;
    
    // Recent Goal History
    SDL_Color historyColor = {255, 150, 100, 255};
    drawText("RECENT GOALS:", m_screenWidth - 340, yPos, historyColor);
    yPos += 15;
    
    const auto& history = g_sifAI->getGoalHistory();
    int historyCount = 0;
    for (auto it = history.rbegin(); it != history.rend() && historyCount < 5; ++it, ++historyCount) {
        ss.str("");
        ss << "  " << std::fixed << std::setprecision(1) << it->timestamp << "s: " << it->goalName;
        drawText(ss.str(), m_screenWidth - 330, yPos, historyColor);
        yPos += 13;
        
        drawText("    -> " + it->reason, m_screenWidth - 320, yPos, {200, 150, 100, 255});
        yPos += 15;
        
        if (yPos > 480) break; // Don't overflow the panel
    }
    
    // Instructions
    SDL_Color instructionColor = {150, 150, 150, 255};
    drawText("Ctrl+D: Toggle Debug", m_screenWidth - 340, m_screenHeight - 40, instructionColor);
    drawText("Ctrl+E: Toggle Enhanced", m_screenWidth - 340, m_screenHeight - 25, instructionColor);
}
