#ifndef GAME_UNITS_H
#define GAME_UNITS_H

#include "Vector2D.h"

namespace GameUnits {
    constexpr float PIXELS_PER_METER = 30.0f;
    
    // Conversion functions
    inline float toMeters(float pixels) { return pixels / PIXELS_PER_METER; }
    inline float toPixels(float meters) { return meters * PIXELS_PER_METER; }
    
    inline Vector2D toMeters(const Vector2D& pixels) {
        return Vector2D(pixels.x / PIXELS_PER_METER, pixels.y / PIXELS_PER_METER);
    }
    
    inline Vector2D toPixels(const Vector2D& meters) {
        return Vector2D(meters.x * PIXELS_PER_METER, meters.y * PIXELS_PER_METER);
    }
}

#endif
