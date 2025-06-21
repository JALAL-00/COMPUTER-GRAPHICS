//
//  Renderer.h
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#pragma once
#include <string>
class Renderer {
public:
    struct Color {
        float r, g, b;
    };
    void drawRect(float x, float y, float width, float height, Color color);
    void drawCircle(float cx, float cy, float radius, Color color, int segments = 32);
    void drawText(float x, float y, const std::string& text, Color color);
};
