#include "DebugDrawUtils.h"

namespace DebugUtils {
    // 坐标变换
    sf::Vector2f ToScreen(const b2Vec2& v) {
        return {
            v.x * SCALE * DEBUG_DRAW_SCALE + DEBUG_DRAW_OFFSET.x,
            v.y * SCALE * DEBUG_DRAW_SCALE + DEBUG_DRAW_OFFSET.y
        };
    }

    // 多边形轮廓
    void DrawPolygonOutline(const b2Vec2* vertices, int count, b2HexColor, void* ctxVoid) {
        auto* ctx = static_cast<DrawContext*>(ctxVoid);
        sf::ConvexShape shape(count);
        for (int i = 0; i < count; ++i)
            shape.setPoint(i, ToScreen(vertices[i]));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Magenta);
        shape.setOutlineThickness(1.0f);
        ctx->window->draw(shape);
    }

    // 多边形填充
    void DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int count, float, b2HexColor, void* ctxVoid) {
        auto* ctx = static_cast<DrawContext*>(ctxVoid);
        auto* debugDraw = ctx->debugDraw;
        sf::ConvexShape shape(count);
        for (int i = 0; i < count; ++i)
            shape.setPoint(i, ToScreen(b2TransformPoint(transform, vertices[i])));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Green);
        shape.setOutlineThickness(1.0f);
        ctx->window->draw(shape);

        if (debugDraw && debugDraw->drawMass && debugDraw->DrawSegmentFcn) {
            b2Vec2 center = transform.p;
            b2Vec2 end = { center.x, center.y + 0.5f };
            debugDraw->DrawSegmentFcn(center, end, b2_colorRed, debugDraw->context);
        }
    }

    // 圆形轮廓
    void DrawCircleOutline(b2Vec2 center, float radius, b2HexColor, void* ctxVoid) {
        auto* ctx = static_cast<DrawContext*>(ctxVoid);
        float r = radius * SCALE * DEBUG_DRAW_SCALE;
        sf::CircleShape shape(r);
        shape.setOrigin({ r, r });
        shape.setPosition(ToScreen(center));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Green);
        shape.setOutlineThickness(1.0f);
        ctx->window->draw(shape);
    }

    // 圆形填充
    void DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* ctxVoid) {
        auto* ctx = static_cast<DrawContext*>(ctxVoid);
        b2DebugDraw* debugDraw = ctx->debugDraw;

        b2Vec2 center = b2TransformPoint(transform, b2Vec2{ 0, 0 });
        DrawCircleOutline(center, radius, color, ctx);

        // 质心绘制逻辑（朝重力方向的小红线段）
        if (debugDraw && debugDraw->drawMass && debugDraw->DrawSegmentFcn) {
            b2Vec2 gravityDir = b2Vec2{ 0, 1 }; // 重力方向
            float len = 0.5f; // 线段长度（米）
            b2Vec2 end = b2Vec2{ center.x + gravityDir.x * len, center.y + gravityDir.y * len };
            debugDraw->DrawSegmentFcn(center, end, b2_colorRed, debugDraw->context); // 红色
        }
    }

    // 线段绘制
    void DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* ctxVoid) {
        auto* ctx = static_cast<DrawContext*>(ctxVoid);
        sf::Color sfColor = sf::Color(
            (color >> 16) & 0xFF,
            (color >> 8) & 0xFF,
            color & 0xFF
        );
        sf::Vertex line[] = {
            { ToScreen(p1), sfColor },
            { ToScreen(p2), sfColor }
        };
        ctx->window->draw(line, 2, sf::PrimitiveType::Lines);
    }

 

    // 初始化调试绘图
    void InitDebugDraw(b2DebugDraw& debugDraw, DrawContext& context) {
        debugDraw.context = &context;
        debugDraw.drawShapes = true; // 绘制刚体形状
        debugDraw.drawGraphColors = true; // 绘制图形颜色
        debugDraw.drawBounds = true;  // 绘制包围盒
        debugDraw.drawMass = true;//绘制动态刚体的质心

        // 必须初始化所有函数指针，哪怕是空函数
        debugDraw.DrawPolygonFcn = DrawPolygonOutline;
        debugDraw.DrawSolidPolygonFcn = DrawSolidPolygon;
        debugDraw.DrawCircleFcn = DrawCircleOutline;
        debugDraw.DrawSolidCircleFcn = DrawSolidCircle;
        debugDraw.DrawSegmentFcn = DrawSegment;

        // 空实现以避免 crash
        debugDraw.DrawSolidCapsuleFcn = [](b2Vec2, b2Vec2, float, b2HexColor, void*) {};
        debugDraw.DrawTransformFcn = [](b2Transform, void*) {};
        debugDraw.DrawPointFcn = [](b2Vec2, float, b2HexColor, void*) {};
        debugDraw.DrawStringFcn = [](b2Vec2, const char*, b2HexColor, void*) {};
    }

}