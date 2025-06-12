#include "InputHandler.h"
#include <SDL2/SDL_scancode.h>
#include <iostream>

InputHandler::InputHandler() : m_keyStates(nullptr), m_attackPressed(false), m_dodgePressed(false) {
    m_keyStates = SDL_GetKeyboardState(nullptr);
}

InputHandler::~InputHandler() {}

void InputHandler::update() {
    // Reset press flags at the start of each frame
    m_attackPressed = false;
    m_dodgePressed = false;
    
    // Update key states for movement (held keys)
    m_keyStates = SDL_GetKeyboardState(nullptr);
}

void InputHandler::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {  // Only on first press, not repeat
        switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_J:
                m_attackPressed = true;
                std::cout << "Attack key pressed via event!" << std::endl;
                break;
            case SDL_SCANCODE_SPACE:
                m_dodgePressed = true;
                std::cout << "Dodge key pressed via event!" << std::endl;
                break;
            default:
                break;
        }
    }
}

bool InputHandler::isKeyDown(SDL_Scancode key) const {
    if (m_keyStates) {
        return m_keyStates[key] == 1;
    }
    return false;
}

bool InputHandler::isKeyPressed(SDL_Scancode key) const {
    if (!m_keyStates) return false;
    
    bool currentState = (m_keyStates[key] == 1);
    bool pressed = currentState;

    return pressed;
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
    return m_attackPressed;
}

bool InputHandler::isDodgePressed() const {
    return m_dodgePressed;
}
