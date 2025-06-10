#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class Player;
class Boss;

class Renderer {
private:
    SDL_Renderer* m_renderer;
    int m_screenWidth;
    int m_screenHeight;
    bool m_debugMode;  // Toggle with D key
    bool m_showAIDebug = true;  // Toggle AI debug specifically

    // Font for text rendering
    TTF_Font* m_font;
    TTF_Font* m_smallFont;

    // Text rendering helper (simple method)
    void drawText(const std::string& text, int x, int y, SDL_Color color);
    void drawTextFallback(const std::string& text, int x, int y, SDL_Color color);
    void drawRect(int x, int y, int w, int h, SDL_Color color, bool filled = false);
    
public:
    Renderer(SDL_Renderer* renderer, int width, int height);
    ~Renderer(); 

    void clear();
    void present();
    
    void drawHealthBar(float x, float y, float width, float height, 
                      float percentage, SDL_Color color);
    void drawStaminaBar(float x, float y, float width, float height, 
                       float percentage);
    void drawEntity(const SDL_Rect& rect, SDL_Color color);
    void drawCircle(int centerX, int centerY, int radius, SDL_Color color);
    void drawUI(const Player* player, const Boss* boss);
    void drawDebugInfo(const Player* player, const Boss* boss);
    void drawAIDebugInfo();
    
    void toggleDebugMode() { m_debugMode = !m_debugMode; }
    void toggleAIDebug() { m_showAIDebug = !m_showAIDebug; }
    bool isDebugMode() const { return m_debugMode; }
    bool isAIDebugMode() const { return m_showAIDebug; }
};

#endif
