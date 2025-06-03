// === InputHandler.h ===
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <SDL2/SDL.h>
#include "Vector2D.h"

class InputHandler {
private:
    const Uint8* m_keyStates;
    
public:
    InputHandler();
    
    void update();
    
    bool isKeyDown(SDL_Scancode key) const;
    Vector2D getMovementDirection() const;
    bool isAttackPressed() const;
    bool isDodgePressed() const;
};

#endif
