#include "Game.h"
#include "GameUnits.h"
#include "Player.h"
#include "Boss.h"
#include "Sif.h"
#include "Renderer.h"
#include "InputHandler.h"
#include <iostream>

// Helper function for AABB collision detection
bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

bool checkCollision(const Circle& a, const SDL_Rect& b) {
    //Closest point on collision box
    float cX, cY;

    //Find closest x offset
    if (a.x < b.x) {
        cX = b.x;
    } else if (a.x > b.x + b.w) {
        cX = b.x + b.w;
    } else {
        cX = a.x;
    }

    //Find closest y offset
    if (a.y < b.y) {
        cY = b.y;
    } else if (a.y > b.y + b.h) {
        cY = b.y + b.h;
    } else {
        cY = a.y;
    }
    
    Vector2D centre{a.x, a.y};
    Vector2D point{cX, cY};

    //If the closest point is inside the circle
    if (centre.distance(point) < a.r) {
        //This box and the circle have collided
        return true;
    }

    //If the shapes have not collided
    return false;
}

// Helper function to push entities apart
void separateEntities(Vector2D& pos1, Vector2D& pos2, float radius1, float radius2) {
    Vector2D diff = pos1 - pos2;
    float distance = diff.length();
    float minDistance = radius1 + radius2;
    
    if (distance < minDistance && distance > 0) {
        // Calculate overlap
        float overlap = minDistance - distance;
        Vector2D pushVector = diff.normalized() * (overlap * 0.5f);
        
        // Push both entities apart equally
        pos1 = pos1 + pushVector;
        pos2 = pos2 - pushVector;
    }
}

Game::Game() : m_isRunning(false), m_window(nullptr), m_renderer(nullptr), m_lastTime(0) {}

Game::~Game() {
    clean();
}

bool Game::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, SDL_WINDOW_SHOWN);
    if (!m_window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize game objects
    m_player = std::make_unique<Player>(width / 2.0f, height * 0.75f);
    m_player->setWindowBounds(width, height);
    m_boss = std::make_unique<Boss>(width / 2.0f, height * 0.25f);
    
    // Initialize Sif AI
    m_sifAI = std::make_unique<HolySwordWolfAI>(m_boss.get(), m_player.get());
    
    m_gameRenderer = std::make_unique<Renderer>(m_renderer, width, height);
    m_inputHandler = std::make_unique<InputHandler>();
    
    m_lastTime = SDL_GetTicks();
    m_isRunning = true;
    
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_isRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_d && event.key.keysym.mod & KMOD_CTRL) {
                m_gameRenderer->toggleDebugMode();  // Ctrl+D for debug mode
            }
            // Add special effect toggle for testing (optional)
            else if (event.key.keysym.sym == SDLK_e && event.key.keysym.mod & KMOD_CTRL) {
                m_sifAI->setEnhanced(!m_sifAI->isEnhanced());  // Ctrl+E to toggle enhanced mode
            }
        }
    }
    
    m_inputHandler->update();
    
    // Handle player input
    Vector2D moveDir = m_inputHandler->getMovementDirection();
    m_player->move(moveDir);
    
    if (m_inputHandler->isAttackPressed()) {
        m_player->attack();
    }
    
    if (m_inputHandler->isDodgePressed() && moveDir.length() > 0) {
        m_player->dodge(moveDir);
    }
}

void Game::update(float deltaTime) {
    if (!m_player->isAlive() || !m_boss->isAlive()) {
        // Game over
        return;
    }
    
    // Update AI first (it will command the boss)
    m_sifAI->update(deltaTime);

    // Store positions before update for collision resolution
    // Vector2D playerPosBeforeUpdate = m_player->getPosition();
    // Vector2D bossPosBeforeUpdate = m_boss->getPosition();

    // Update entities
    m_player->update(deltaTime);
    m_boss->update(deltaTime);

    // Body-to-body collision between player and boss
    SDL_Rect playerBox = m_player->getCollisionBox();
    SDL_Rect bossBox = m_boss->getCollisionBox();
    
    if (checkCollision(playerBox, bossBox)) {
        Vector2D playerPos = m_player->getPosition();
        Vector2D bossPos = m_boss->getPosition();
        
        // Calculate half dimensions
        float playerHalfW = m_player->getWidth() / 2.0f;
        float playerHalfH = m_player->getHeight() / 2.0f;
        float bossHalfW = m_boss->getWidth() / 2.0f;
        float bossHalfH = m_boss->getHeight() / 2.0f;
        
        // Calculate the distance between centers
        float dx = playerPos.x - bossPos.x;
        float dy = playerPos.y - bossPos.y;
        
        // Calculate overlap on each axis
        float overlapX = (playerHalfW + bossHalfW) - std::abs(dx);
        float overlapY = (playerHalfH + bossHalfH) - std::abs(dy);
        
        Vector2D newPlayerPos = playerPos;
        
        // Push along the axis with smallest overlap
        if (overlapX < overlapY) {
            // Push horizontally
            if (dx > 0) {
                // Player is to the right of boss, push right
                newPlayerPos.x += overlapX;
            } else {
                // Player is to the left of boss, push left
                newPlayerPos.x -= overlapX;
            }
        } else {
            // Push vertically
            if (dy > 0) {
                // Player is below boss, push down
                newPlayerPos.y += overlapY;
            } else {
                // Player is above boss, push up
                newPlayerPos.y -= overlapY;
            }
        }
        
        // Keep player in bounds
        newPlayerPos.x = std::max(playerHalfW, std::min(GameUnits::toMeters(800.0f) - playerHalfW, newPlayerPos.x));
        newPlayerPos.y = std::max(playerHalfH, std::min(GameUnits::toMeters(600.0f) - playerHalfH, newPlayerPos.y));
        
        // Only update player position
        m_player->setPosition(newPlayerPos);
    }

    // Check sword collisions
    // Player sword attack vs Boss body
    if (m_player->getState() == PlayerState::ATTACKING && !m_player->hasDealtDamage()) {
        SDL_Rect playerSwordBox = m_player->getSwordHitbox();
        if (checkCollision(playerSwordBox, bossBox)) {
            float damage = m_player->getAttackDamage();
            m_boss->takeDamage(damage);
            m_player->setDamageDealt();
            
            // Notify AI that boss was damaged
            // m_sifAI->OnDamaged(damage, m_player->getPosition());
        }
    }
    
    // Boss sword attack vs Player body
    if (m_boss->isAttacking() && !m_player->isInvulnerable() && !m_boss->hasDealtDamage()) {
        bool isCollided = false;
        
        // Check appropriate hitbox based on attack type
        Circle attackCircle = m_boss->getAttackCircle();
        isCollided = checkCollision(attackCircle, playerBox);
        
        if (isCollided) {
            m_player->takeDamage(m_boss->getAttackDamage());
            m_boss->setDamageDealt();
        }
    }
    
    // Check for projectiles (if player has projectile attacks)
    // This is a placeholder - implement based on your Player class
    /*
    if (m_player->hasActiveProjectile()) {
        Vector2D projectilePos = m_player->getProjectilePosition();
        m_sifAI->OnProjectileDetected(projectilePos);
    }
    */
}

void Game::render() {
    m_gameRenderer->clear();
    
    m_player->render(m_renderer);
    m_boss->render(m_renderer);
    m_gameRenderer->drawDebugInfo(m_player.get(), m_boss.get());
    m_gameRenderer->drawUI(m_player.get(), m_boss.get());
    
    m_gameRenderer->present();
}

void Game::clean() {
    // Clean up AI
    m_sifAI.reset();
    
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
}
