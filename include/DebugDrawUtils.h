#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
const float SCALE = 30.f;
const float DEG = 57.29577f;

const float DEBUG_DRAW_SCALE = 0.35f;
const sf::Vector2f DEBUG_DRAW_OFFSET(300, 50);

struct DrawContext {
    sf::RenderWindow* window;
    b2DebugDraw* debugDraw;
};

namespace DebugUtils {
    sf::Vector2f ToScreen(const b2Vec2& worldPos);
    void DrawPolygonOutline(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* ctxVoid);
    void DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* ctxVoid);
    void DrawCircleOutline(b2Vec2 center, float radius, b2HexColor color, void* ctxVoid);
    void DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* ctxVoid);
    void DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* ctxVoid);
    void InitDebugDraw(b2DebugDraw& debugDraw, DrawContext& context);
}
