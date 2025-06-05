#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

class Player;
class Boss;

class Renderer {
private:
    SDL_Renderer* m_renderer;
    int m_screenWidth;
    int m_screenHeight;
    bool m_debugMode;  // Toggle with D key
    
public:
    Renderer(SDL_Renderer* renderer, int width, int height);
    
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
    
    void toggleDebugMode() { m_debugMode = !m_debugMode; }
    bool isDebugMode() const { return m_debugMode; }
};

#endif
