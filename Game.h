#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <memory>

class Player;
class Boss;
class Renderer;
class InputHandler;
class HolySwordWolfAI;

class Game {
private:
    bool m_isRunning;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Boss> m_boss;
    std::unique_ptr<HolySwordWolfAI> m_sifAI;  // Added Sif AI
    std::unique_ptr<Renderer> m_gameRenderer;
    std::unique_ptr<InputHandler> m_inputHandler;
    
    Uint32 m_lastTime;
    
public:
    Game();
    ~Game();
    
    bool init(const char* title, int width, int height);
    void handleEvents();
    void update(float deltaTime);
    void render();
    void clean();
    
    bool isRunning() const { return m_isRunning; }
    void quit() { m_isRunning = false; }
};

#endif
