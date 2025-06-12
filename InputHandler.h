#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <SDL2/SDL.h>
#include "Vector2D.h"

class InputHandler {
    const Uint8* m_keyStates;

    // Event-based key press detection
    bool m_attackPressed;
    bool m_dodgePressed;
    
public:
    InputHandler();
    ~InputHandler();
    
    void update();
    void handleEvent(const SDL_Event& event);  // NEW: Handle SDL events
    
    bool isKeyDown(SDL_Scancode key) const;
    bool isKeyPressed(SDL_Scancode key) const;  
    Vector2D getMovementDirection() const;
    bool isAttackPressed() const;
    bool isDodgePressed() const;
};

#endif
