#include "Game.h"
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <iostream>
#include "Timer.h"

int main() {
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    const int FPS = 60;  // Increased to 60 FPS for smoother combat
    const int frameDelay = 1000 / FPS;
    
    Game game;
    
    if (!game.init("Dark Souls 2D - Sif Boss Fight", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }
    
    Timer fpsTimer, capTimer;
    int countedFrames = 0;
    Uint32 previousTime = 0;
    Uint32 currentTime = 0;
    float deltaTime = 0.0f;

    fpsTimer.start();
    previousTime = SDL_GetTicks();

    while (game.isRunning()) {
        capTimer.start();

        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        // Cap delta time to prevent huge jumps (if game was paused, etc.)
        if (deltaTime > 0.05f) {  // Max 50ms = 20 FPS minimum
            deltaTime = 0.05f;
        }

        // FPS counter (only print every 60 frames to reduce console spam)
        if (countedFrames % 60 == 0) {
            float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
            std::cout << "FPS: " << avgFPS << std::endl;
        }

        game.handleEvents();
        game.update(deltaTime);
        game.render();

        ++countedFrames;
        
        int frameTime = capTimer.getTicks();
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }
    
    return 0;
}
