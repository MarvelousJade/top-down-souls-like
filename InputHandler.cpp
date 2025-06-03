// === InputHandler.cpp ===
#include "InputHandler.h"
#include <SDL2/SDL_scancode.h>

InputHandler::InputHandler() : m_keyStates(nullptr) {}

void InputHandler::update() {
    m_keyStates = SDL_GetKeyboardState(nullptr);
}

bool InputHandler::isKeyDown(SDL_Scancode key) const {
    if (m_keyStates) {
        return m_keyStates[key] == 1;
    }
    return false;
}

Vector2D InputHandler::getMovementDirection() const {
    Vector2D direction(0, 0);
    
    if (isKeyDown(SDL_SCANCODE_W)) direction.y -= 1;
    if (isKeyDown(SDL_SCANCODE_S)) direction.y += 1;
    if (isKeyDown(SDL_SCANCODE_A)) direction.x -= 1;
    if (isKeyDown(SDL_SCANCODE_D)) direction.x += 1;
    
    return direction.normalized();
}

bool InputHandler::isAttackPressed() const {
    return isKeyDown(SDL_SCANCODE_J);
}

bool InputHandler::isDodgePressed() const {
    return isKeyDown(SDL_SCANCODE_SPACE);
}
