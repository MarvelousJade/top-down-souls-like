#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

class LTexture {
public:
    // Initializes variables
    LTexture();

    // Deallocates memory
    ~LTexture();

    // Loads image at specified path
    bool loadFromFile(std::string path);

    // Deallocates texture
    void free();

    // Set color modulation
    void setColor(Uint8 red, Uint8 green, Uint8 blue);

    // Set blending
    void setBlendMode(SDL_BlendMode blending);

    // Set alpha modulation
    void setAlpha(Uint8 alpha);
    
    // Renders texture at given point
    void render(int x, int y, SDL_Rect* clip = NULL);

    // Gets image dimensions
    int getWidth();
    int getHeight();

    // Set the global renderer (call this once during initialization)
    static void setRenderer(SDL_Renderer* renderer);

private:
    // The actual hardware texture
    SDL_Texture* mTexture;

    // Image dimensions
    int mWidth;
    int mHeight;
    
    // Global renderer for all LTexture instances
    static SDL_Renderer* s_Renderer;
};

#endif
