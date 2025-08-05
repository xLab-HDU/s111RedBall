#pragma once
#include "Game.h"

sf::Vector2f ToScreen(const b2Vec2& worldPos);

void DrawPolygonOutline(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* ctxVoid);
void DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* ctxVoid);
void DrawCircleOutline(b2Vec2 center, float radius, b2HexColor color, void* ctxVoid);
void DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* ctxVoid);
void DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* ctxVoid);

void CreateStaticBox(b2WorldId world, float x, float y, float w, float h);
void InitDebugDraw(b2DebugDraw& debugDraw, DrawContext& context);
