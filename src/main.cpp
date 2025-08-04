#include <iostream>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

using namespace sf;

const float SCALE = 30.f;		//MKS物理单位到像素单位的换算因子，表示每米使用30个像素
const float DEG = 57.29577f;//1弧度等于DEG对应的角度数

const float DEBUG_DRAW_SCALE = 0.35f; // 缩小比例
const sf::Vector2f DEBUG_DRAW_OFFSET(300, 50); // y轴向下偏移50像素

Texture t, t1, t2, t3, t4, t5, t6;
Sprite sGround(t), sPlayer(t1), sBox(t2), sStone(t2), sSwing(t3), sPlatform(t4), sFlag(t5), sBackground(t6);

int jumpCooldown = 0;
bool groundShowFlag = true;
bool debugDrawFlag = false;
bool isBodyContact = false;
bool isOnGround = false;

// 用户上下文，用于传递 SFML 窗口
struct DrawContext {
    sf::RenderWindow* window;
    b2DebugDraw* debugDraw;
};

void LoadMediaData()
{
    if (!t.loadFromFile("../data/images/ground.png")) {
        std::cout << "Failed to load ground texture!" << std::endl;
    }
    if (!t1.loadFromFile("../data/images/player.png")) {
        std::cout << "Failed to load player texture!" << std::endl;
    }
    if (!t2.loadFromFile("../data/images/objects1.png")) {
        std::cout << "Failed to load objects1 texture!" << std::endl;
    }
    if (!t3.loadFromFile("../data/images/objects2.png")) {
        std::cout << "Failed to load objects2 texture!" << std::endl;
    }
    if (!t4.loadFromFile("../data/images/platform.png")) {
        std::cout << "Failed to load platform texture!" << std::endl;
    }
    if (!t5.loadFromFile("../data/images/flag.png")) {
        std::cout << "Failed to load flag texture!" << std::endl;
    }
    if (!t6.loadFromFile("../data/images/back.png")) {
        std::cout << "Failed to load background texture!" << std::endl;
    }

    t1.setSmooth(true);//因为要做物理接触，这3个对象的纹理，做下模糊处理
    t2.setSmooth(true);
    t3.setSmooth(true);

    sGround.setTexture(t, true);
    sPlayer.setTexture(t1, true);
    sBox.setTexture(t2, true);
    sStone.setTexture(t2, true);
    sSwing.setTexture(t3, true);
    sPlatform.setTexture(t4, true);
    sFlag.setTexture(t5, true);
    sBackground.setTexture(t6, true);

    sBox.setTextureRect(IntRect({ 81,0 }, { 61,61 }));
    sBox.setOrigin({ 30,30 });

    sPlayer.setTextureRect(IntRect({ 0,160 }, { 32,32 }));
    sPlayer.setOrigin({ 16,16 });

    sSwing.setTextureRect(IntRect({ 378,0 }, { 28,242 }));
    sSwing.setOrigin({ 20,121 });

    sStone.setTextureRect(IntRect({ 0,0 }, { 80,80 }));
    sStone.setOrigin({ 40,40 });

    sGround.setPosition({ 0,320 });
    sFlag.setPosition({ 587,19 });
}

void SetWall(int x, int y, int w, int h, b2WorldId myWorldId)
{
    b2BodyDef bdef = b2DefaultBodyDef();
    bdef.enableSleep = true; //设置物理对象可以休眠
    bdef.isAwake = true; //设置物理对象处于激活状态
    bdef.position = b2Vec2{ x / SCALE, y / SCALE };
    b2BodyId groundId = b2CreateBody(myWorldId, &bdef);
    b2Polygon groundBox = b2MakeBox(w / SCALE, h / SCALE);//实例化一个多边形对象,半个宽度和半个高度作为参数
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
}

// 物理坐标转缩放后窗口坐标
sf::Vector2f DebugDraw_Transform(const b2Vec2& v) {
    return sf::Vector2f(
        v.x * SCALE * DEBUG_DRAW_SCALE + DEBUG_DRAW_OFFSET.x,
        v.y * SCALE * DEBUG_DRAW_SCALE + DEBUG_DRAW_OFFSET.y
    );
}

// 多边形绘制（无变换）
void DebugDraw_Polygon(const b2Vec2* vertices, int vertexCount, b2HexColor, void* ctxVoid) {
    auto* ctx = static_cast<DrawContext*>(ctxVoid);
    sf::ConvexShape shape(vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
        shape.setPoint(i, DebugDraw_Transform(vertices[i]));
    }
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Magenta);
    shape.setOutlineThickness(1.0f);
    ctx->window->draw(shape);
}

// 多边形绘制（带变换，用b2TransformPoint）
void DebugDraw_SolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* ctxVoid) {
    auto* ctx = static_cast<DrawContext*>(ctxVoid);
    b2DebugDraw* debugDraw = ctx->debugDraw; // 通过 ctx 获取 debugDraw

    sf::ConvexShape shape(vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
        b2Vec2 worldPt = b2TransformPoint(transform, vertices[i]);
        shape.setPoint(i, DebugDraw_Transform(worldPt));
    }
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Green);
    shape.setOutlineThickness(1.0f);
    ctx->window->draw(shape);

    // 质心绘制逻辑（朝重力方向的小红线段）
    if (debugDraw && debugDraw->drawMass && debugDraw->DrawSegmentFcn) {
        b2Vec2 center = transform.p; // 质心坐标
        b2Vec2 gravityDir = b2Vec2{ 0, 1 }; // 重力方向
        float len = 0.5f; // 线段长度（米）
        b2Vec2 end = b2Vec2{ center.x + gravityDir.x * len, center.y + gravityDir.y * len };
        debugDraw->DrawSegmentFcn(center, end, b2_colorRed, debugDraw->context); // 红色
    }
}

// 圆形绘制（无变换）
void DebugDraw_Circle(b2Vec2 center, float radius, b2HexColor, void* ctxVoid) {
    auto* ctx = static_cast<DrawContext*>(ctxVoid);
    float scaledRadius = radius * SCALE * DEBUG_DRAW_SCALE;
    sf::CircleShape shape(scaledRadius);
    shape.setOrigin({ scaledRadius, scaledRadius });
    shape.setPosition(DebugDraw_Transform(center));
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Green);
    shape.setOutlineThickness(1.0f);
    ctx->window->draw(shape);
}

// 圆形绘制（带变换）
void DebugDraw_SolidCircle(b2Transform transform, float radius, b2HexColor color, void* ctxVoid) {
    auto* ctx = static_cast<DrawContext*>(ctxVoid);
    b2DebugDraw* debugDraw = ctx->debugDraw;

    b2Vec2 center = b2TransformPoint(transform, b2Vec2{ 0, 0 });
    DebugDraw_Circle(center, radius, color, ctx);

    // 质心绘制逻辑（朝重力方向的小红线段）
    if (debugDraw && debugDraw->drawMass && debugDraw->DrawSegmentFcn) {
        b2Vec2 gravityDir = b2Vec2{ 0, 1 }; // 重力方向
        float len = 0.5f; // 线段长度（米）
        b2Vec2 end = b2Vec2{ center.x + gravityDir.x * len, center.y + gravityDir.y * len };
        debugDraw->DrawSegmentFcn(center, end, b2_colorRed, debugDraw->context); // 红色
    }
}

// 线段绘制（带变换）
void DebugDraw_Segment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* ctxVoid) {
    auto* ctx = static_cast<DrawContext*>(ctxVoid);
    sf::Color sfColor = sf::Color(
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF
    );
    sf::Vertex line[] = {
        { DebugDraw_Transform(p1), sfColor },
        { DebugDraw_Transform(p2), sfColor }
    };
    ctx->window->draw(line, 2, sf::PrimitiveType::Lines);
}

bool ContactDetect(b2WorldId myWorldId, b2BodyId myBodyId)
{
    b2ContactEvents contactEvents = b2World_GetContactEvents(myWorldId);
    //  遍历所有接触事件，如果有一个body是playerBody，说明玩家对象与其他物体发生碰撞
    for (int i = 0; i < contactEvents.beginCount; i++) {
        b2ContactBeginTouchEvent e = contactEvents.beginEvents[i];
        b2ShapeId shapeA = e.shapeIdA;
        b2ShapeId shapeB = e.shapeIdB;
        b2BodyId bodyA = b2Shape_GetBody(shapeA);
        b2BodyId bodyB = b2Shape_GetBody(shapeB);
        // 检查是否涉及 playerBody
        if (B2_ID_EQUALS(bodyA, myBodyId) || B2_ID_EQUALS(bodyB, myBodyId)) {
            isBodyContact = true;
        }
    }
    for (int i = 0; i < contactEvents.endCount; i++) {
        b2ContactEndTouchEvent e = contactEvents.endEvents[i];
        b2ShapeId shapeA = e.shapeIdA;
        b2ShapeId shapeB = e.shapeIdB;
        b2BodyId bodyA = b2Shape_GetBody(shapeA);
        b2BodyId bodyB = b2Shape_GetBody(shapeB);
        // 检查是否涉及 playerBody
        if (B2_ID_EQUALS(bodyA, myBodyId) || B2_ID_EQUALS(bodyB, myBodyId)) {
            isBodyContact = false;
        }
    }
    return isBodyContact;
}

void Input(b2BodyId& myBodyId, sf::RenderWindow& myWindow)
{
    b2Vec2 vel = b2Body_GetLinearVelocity(myBodyId);//获取物理对象的线速度
    float angVel = b2Body_GetAngularVelocity(myBodyId);//获取物理对象的角速度
    if (Keyboard::isKeyPressed(Keyboard::Scancode::Right))
        if (vel.x < 20)
        {
            b2Body_ApplyForceToCenter(myBodyId, b2Vec2({ 40.0f, 0.0f }), true);//应用一个力到物理对象的中心
            if (angVel < 15)
                b2Body_ApplyTorque(myBodyId, 10, true);//应用一个扭矩到物理对象
        }
    if (Keyboard::isKeyPressed(Keyboard::Scancode::Left))
        if (vel.x > -20)
        {
            b2Body_ApplyForceToCenter(myBodyId, b2Vec2({ -40.0f, 0.0f }), true);
            if (angVel > -15)
                b2Body_ApplyTorque(myBodyId, -10, true);
        }

    if (Keyboard::isKeyPressed(Keyboard::Scancode::Up))
    {
        if (isOnGround && jumpCooldown == 0)
        {
            b2Body_ApplyForceToCenter(myBodyId, b2Vec2({ 0.0f, -1200.0f }), true);//向上施加一个力，模拟跳跃
            jumpCooldown = 15;
        }
    }
    if (jumpCooldown > 0) jumpCooldown--;
    while (const std::optional event = myWindow.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            myWindow.close();
        }

        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            if (keyReleased->scancode == sf::Keyboard::Scancode::Enter)
            {
                groundShowFlag = !groundShowFlag;
            }
            if (keyReleased->scancode == sf::Keyboard::Scancode::Space)
            {
                debugDrawFlag = !debugDrawFlag;
            }
        }
    }
}

void DrawBody(b2BodyId myBodyId, Sprite mySprite, sf::RenderWindow& myWindow)
{
    b2Vec2 boxPos = b2Body_GetPosition(myBodyId);
    b2Rot rotation = b2Body_GetRotation(myBodyId);
    float angle = b2Rot_GetAngle(rotation);//获取物理对象的角度
    mySprite.setPosition({ boxPos.x * SCALE, boxPos.y * SCALE });//*SCALE的原因是将Box2D的单位米，转为图形绘制的单位像素
    mySprite.setRotation(sf::degrees(angle * DEG));				//*DEG的原因是将Box2D的单位弧度，转为图形绘制的单位度
    myWindow.draw(mySprite);
}

int main()
{
    // 初始化 World
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{ 0.0f, 9.8f };
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Request a 24-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 24;
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }, 32), "Red Ball C++", sf::Style::Default, sf::State::Windowed, contextSettings);//Since SFML is based on OpenGL, its windows are ready for OpenGL calls without any extra effort.
    window.setFramerateLimit(60);

    LoadMediaData();

    /////////box2d///////////数值的单位为像素，函数内容会再转为米
    SetWall(400, 490, 2000, 10, worldId);//地面
    SetWall(400, 0, 2000, 1, worldId);//天花板
    SetWall(55, 438, 64, 60, worldId);//左边中高台
    SetWall(710, 435, 100, 60, worldId);//右边中高台
    SetWall(158, 224, 33, 10, worldId);//放石头的台子
    SetWall(608, 114, 33, 10, worldId);//插旗子的高台

    // 几何中心坐标为(0,0)
    b2Vec2 vertices[5] = {
        {-1.0f, -0.8f},
        {1.0f, -0.8f},
        {1.0f, 0.2f},
        {0.0f, 1.2f},
        {-1.0f, 0.2f}
    };
    b2Hull hull = b2ComputeHull(vertices, 5);
    b2Polygon polygon = b2MakePolygon(&hull, 0);

    b2Polygon shape = b2MakeBox(30 / SCALE, 30 / SCALE);//创建一个形状

    b2ShapeDef sdef = b2DefaultShapeDef();
    sdef.density = 1.0f;
    sdef.material.restitution = 0.0f;//设置物理对象的材质属性，将弹性系数设为0，只依赖自定义跳跃
    sdef.isSensor = false;
    sdef.enableContactEvents = true;

    b2BodyDef bdef = b2DefaultBodyDef();//预定义一个物理对象，或者说设置物理对象的属性
    bdef.type = b2_dynamicBody;			//活动的对象，或受力作用的对象
    bdef.position = b2Vec2{ 21, 10 };//设置物理对象的初始位置
    bdef.enableSleep = true; //设置物理对象可以休眠
    bdef.isAwake = true; //设置物理对象处于激活状态

    b2BodyId body0 = b2CreateBody(worldId, &bdef);
    b2ShapeId shapeId0 = b2CreatePolygonShape(body0, &sdef, &polygon);

    ///////////////////////
    sdef.density = 2.0f;
    bdef.position = b2Vec2{ 18,10 };//设置物理对象的初始位置
    b2BodyId boxBody1 = b2CreateBody(worldId, &bdef);
    b2ShapeId boxShapeId1 = b2CreatePolygonShape(boxBody1, &sdef, &shape);

    bdef.position = b2Vec2{ 18,6 };
    b2BodyId boxBody2 = b2CreateBody(worldId, &bdef);
    b2ShapeId boxShapeId2 = b2CreatePolygonShape(boxBody2, &sdef, &shape);

    bdef.position = b2Vec2{ 5, 14 };
    b2BodyId boxBody3 = b2CreateBody(worldId, &bdef);
    b2ShapeId boxShapeId3 = b2CreatePolygonShape(boxBody3, &sdef, &shape);

    ///////////////////////
    sdef.density = 2.0f;
    bdef.position = b2Vec2{ 2, 2 };
    b2Circle circle;
    circle.radius = 16 / SCALE;
    b2BodyId playerBody = b2CreateBody(worldId, &bdef);
    b2ShapeId playerShapeId = b2CreateCircleShape(playerBody, &sdef, &circle);

    /////////////////////////
    sdef.density = 8.0f;
    bdef.position = b2Vec2{ 5, 1 };
    circle.radius = 40 / SCALE;
    b2BodyId stoneBody = b2CreateBody(worldId, &bdef);
    b2ShapeId stoneShapeId = b2CreateCircleShape(stoneBody, &sdef, &circle);

    /////////////////////////
    sdef.density = 1.0f;
    bdef.position = b2Vec2{ 11, 15 };
    bdef.rotation = b2MakeRot(270 / DEG);//设置物理对象的初始角度
    circle.radius = 40 / SCALE;
    b2BodyId swingBody = b2CreateBody(worldId, &bdef);
    shape = b2MakeBox(5 / SCALE, 120 / SCALE);//创建一个长方形的形状
    b2ShapeId swingShapeId1 = b2CreatePolygonShape(swingBody, &sdef, &shape);//创建一个多边形形状
    circle.radius = 2 / SCALE;//设置圆形的半径
    circle.center = b2Vec2{ -20 / SCALE, 0 };//设置圆形的中心点
    b2ShapeId swingShapeId2 = b2CreateCircleShape(swingBody, &sdef, &circle);//创建一个圆形形状

    // 注册绘制回调
    b2DebugDraw debugDraw{};
    DrawContext ctx{ &window, &debugDraw }; // 传递 debugDraw 指针
    debugDraw.context = &ctx;

    debugDraw.drawShapes = true; // 绘制形状
    debugDraw.drawGraphColors = true; // 绘制图形颜色
    debugDraw.drawBounds = true;  // 绘制包围盒
    debugDraw.drawMass = true;//绘制动态物体的质量和质心

    // 必须初始化所有函数指针，哪怕是空函数
    debugDraw.DrawPolygonFcn = DebugDraw_Polygon;
    debugDraw.DrawSolidPolygonFcn = DebugDraw_SolidPolygon;
    debugDraw.DrawCircleFcn = DebugDraw_Circle;
    debugDraw.DrawSolidCircleFcn = DebugDraw_SolidCircle;
    debugDraw.DrawSegmentFcn = DebugDraw_Segment;

    // 空实现以避免 crash
    debugDraw.DrawSolidCapsuleFcn = [](b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {};
    debugDraw.DrawTransformFcn = [](b2Transform, void*) {};
    debugDraw.DrawPointFcn = [](b2Vec2, float, b2HexColor, void*) {};
    debugDraw.DrawStringFcn = [](b2Vec2 p, const char* s, b2HexColor color, void* context) {};

    while (window.isOpen())
    {
        //Box2D世界的模拟循环开始
        b2World_Step(worldId, 1.0f / 60.f, 4);

        isOnGround = ContactDetect(worldId, playerBody);

        Input(playerBody, window);//处理用户输入

        //////////Draw///////////////
        window.clear(sf::Color::Black); // 在渲染绘制之前，调用clear进行清屏
        if (debugDrawFlag == true)
            b2World_Draw(worldId, &debugDraw);//绘制Box2D图形内容
        if (groundShowFlag == true)
            window.draw(sBackground);
        window.draw(sGround);
        window.draw(sFlag);

        sPlatform.setPosition({ 100, 200 });
        window.draw(sPlatform);
        sPlatform.setPosition({ 550, 90 });
        window.draw(sPlatform);

        //绘制Box2D的物理对象
        DrawBody(boxBody1, sBox, window);
        DrawBody(boxBody2, sBox, window);
        DrawBody(boxBody3, sBox, window);
        DrawBody(playerBody, sPlayer, window);
        DrawBody(stoneBody, sStone, window);
        DrawBody(swingBody, sSwing, window);
        window.display();
    }
    b2DestroyWorld(worldId);//销毁Box2D物理世界
    return 0;
}
