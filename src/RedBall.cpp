// RedBall.cpp
#include "RedBall.h"

#include <iostream>

RedBall::RedBall()
    : contextSettings(),
    ctx{ &window, &debugDraw },
    textures(),
    sGround(textures[0]), sPlayer(textures[1]), sBox(textures[2]), sStone(textures[2]),
    sSwing(textures[3]), sPlatform(textures[4]), sFlag(textures[5]), sBackground(textures[6])
{
    contextSettings.depthBits = 24;
    window.create(sf::VideoMode({ 800, 600 }, 32), "Red Ball C++", sf::Style::Default, sf::State::Windowed, contextSettings);
    window.setFramerateLimit(60);

    loadTextures();
    createSprites();
    createWorld();
    createObjects();

    DebugUtils::InitDebugDraw(debugDraw, ctx);

}

void RedBall::run() {
    while (window.isOpen()) {
        b2World_Step(worldId, 1.0f / 60.f, 4);
        contactDetect();
        handleInput();
        drawScene();
    }
    b2DestroyWorld(worldId);
}

void RedBall::loadTextures() {
    const char* files[] = {
        "../data/images/ground.png",
        "../data/images/player.png",
        "../data/images/objects1.png",
        "../data/images/objects2.png",
        "../data/images/platform.png",
        "../data/images/flag.png",
        "../data/images/back.png"
    };

    for (int i = 0; i < 7; ++i) {
        if (!textures[i].loadFromFile(files[i])) {
            std::cerr << "Failed to load texture: " << files[i] << std::endl;
        }
    }
    textures[1].setSmooth(true);
    textures[2].setSmooth(true);
    textures[3].setSmooth(true);
}

void RedBall::createSprites() {
    sGround.setTexture(textures[0], true);
    sPlayer.setTexture(textures[1], true);
    sBox.setTexture(textures[2], true);
    sStone.setTexture(textures[2], true);
    sSwing.setTexture(textures[3], true);
    sPlatform.setTexture(textures[4], true);
    sFlag.setTexture(textures[5], true);
    sBackground.setTexture(textures[6], true);

    sBox.setTextureRect({ {81, 0}, {61, 61} });
    sBox.setOrigin({ 30, 30 });
    sPlayer.setTextureRect({ {0, 160}, {32, 32} });
    sPlayer.setOrigin({ 16, 16 });
    sSwing.setTextureRect({ {378, 0}, {28, 242} });
    sSwing.setOrigin({ 20, 121 });
    sStone.setTextureRect({ {0, 0}, {80, 80} });
    sStone.setOrigin({ 40, 40 });
    sGround.setPosition({ 0, 320 });
    sFlag.setPosition({ 587, 19 });
}

void RedBall::createWorld() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, 9.8f };
    worldId = b2CreateWorld(&worldDef);
}

void RedBall::setWall(int x, int y, int w, int h, b2WorldId worldId) {
    b2Polygon shape = b2MakeBox(w / SCALE, h / SCALE);//创建一个形状
    b2ShapeDef sdef = b2DefaultShapeDef();//初始化一个默认的形状定义
    b2BodyDef bdef = b2DefaultBodyDef();//初始化一个默认的刚体定义
    bdef.position = b2Vec2{ x / SCALE, y / SCALE };//设置刚体的世界坐标位置
    b2BodyId bodyId = b2CreateBody(worldId, &bdef);//在世界中创建一个刚体
    b2CreatePolygonShape(bodyId, &sdef, &shape);//将形状shape与形状定义sdef附着到该刚体上
}

void RedBall::createObjects() {
    /////////box2d///////////数值的单位为像素，函数内容会再转为米
    setWall(400, 490, 2000, 10, worldId);//地面
    setWall(400, 0, 2000, 1, worldId);//天花板
    setWall(55, 438, 64, 60, worldId);//左边中高台
    setWall(710, 435, 100, 60, worldId);//右边中高台
    setWall(158, 224, 33, 10, worldId);//放石头的台子
    setWall(608, 114, 33, 10, worldId);//插旗子的高台

    // 定义一个点集，作为多边形的顶点
    // 几何中心坐标为(0,0)
    b2Vec2 vertices[5] = {
        {-1.0f, -0.8f},
        {1.0f, -0.8f},
        {1.0f, 0.2f},
        {0.0f, 1.2f},
        {-1.0f, 0.2f}
    };
    b2Hull hull = b2ComputeHull(vertices, 5);//计算点集中所有点的凸包，移除共线点或近重合点
    b2Polygon polygon = b2MakePolygon(&hull, 0);//创建一个多边形形状

    b2Polygon shape = b2MakeBox(30 / SCALE, 30 / SCALE);//创建一个形状

    b2ShapeDef sdef = b2DefaultShapeDef();//使用默认形状初始化
    sdef.density = 1.0f;//设置密度为1.0
    sdef.material.restitution = 0.0f;//设置刚体的材质属性，将弹性系数设为0，只依赖自定义跳跃
    sdef.enableContactEvents = true;//开启接触事件

    b2BodyDef bdef = b2DefaultBodyDef();//初始化一个默认的刚体定义
    bdef.type = b2_dynamicBody;//类型设为动态
    bdef.position = b2Vec2{ 21, 10 };//设置刚体的初始位置

    body0 = b2CreateBody(worldId, &bdef);//在指定的物理世界中创建刚体
    b2ShapeId shapeId0 = b2CreatePolygonShape(body0, &sdef, &polygon);//将已生成的多边形附加到刚体body0上

    ///////////////////////
    sdef.density = 2.0f;
    bdef.position = b2Vec2{ 18,10 };//设置物理对象的初始位置
    boxBody1 = b2CreateBody(worldId, &bdef);
    b2ShapeId boxShapeId1 = b2CreatePolygonShape(boxBody1, &sdef, &shape);

    bdef.position = b2Vec2{ 18,6 };
    boxBody2 = b2CreateBody(worldId, &bdef);
    b2ShapeId boxShapeId2 = b2CreatePolygonShape(boxBody2, &sdef, &shape);

    bdef.position = b2Vec2{ 5, 14 };
    boxBody3 = b2CreateBody(worldId, &bdef);
    b2ShapeId boxShapeId3 = b2CreatePolygonShape(boxBody3, &sdef, &shape);

    ///////////////////////
    sdef.density = 2.0f;
    bdef.position = b2Vec2{ 2, 2 };
    b2Circle circle;
    circle.radius = 16 / SCALE;
    playerBody = b2CreateBody(worldId, &bdef);
    b2ShapeId playerShapeId = b2CreateCircleShape(playerBody, &sdef, &circle);

    /////////////////////////
    sdef.density = 8.0f;
    bdef.position = b2Vec2{ 5, 1 };
    circle.radius = 40 / SCALE;
    stoneBody = b2CreateBody(worldId, &bdef);
    b2ShapeId stoneShapeId = b2CreateCircleShape(stoneBody, &sdef, &circle);

    /////////////////////////
    sdef.density = 1.0f;
    bdef.position = b2Vec2{ 11, 15 };
    bdef.rotation = b2MakeRot(270 / DEG);//设置物理对象的初始角度
    circle.radius = 40 / SCALE;
    swingBody = b2CreateBody(worldId, &bdef);
    shape = b2MakeBox(5 / SCALE, 120 / SCALE);//创建一个长方形的形状
    b2ShapeId swingShapeId1 = b2CreatePolygonShape(swingBody, &sdef, &shape);//创建一个多边形形状
    circle.radius = 2 / SCALE;//设置圆形的半径
    circle.center = b2Vec2{ -20 / SCALE, 0 };//设置圆形的中心点
    b2ShapeId swingShapeId2 = b2CreateCircleShape(swingBody, &sdef, &circle);//创建一个圆形形状
}

void RedBall::contactDetect() {
    b2ContactEvents contactEvents = b2World_GetContactEvents(worldId);
    //  遍历所有接触事件，如果有一个body是playerBody，说明玩家对象与其他物体发生碰撞
    for (int i = 0; i < contactEvents.beginCount; i++) {
        b2ContactBeginTouchEvent e = contactEvents.beginEvents[i];
        if (B2_ID_EQUALS(b2Shape_GetBody(e.shapeIdA), playerBody) ||
            B2_ID_EQUALS(b2Shape_GetBody(e.shapeIdB), playerBody)) {
            footContacts = 1;
        }
    }
    for (int i = 0; i < contactEvents.endCount; i++) {
        b2ContactEndTouchEvent e = contactEvents.endEvents[i];
        if (B2_ID_EQUALS(b2Shape_GetBody(e.shapeIdA), playerBody) ||
            B2_ID_EQUALS(b2Shape_GetBody(e.shapeIdB), playerBody)) {
            footContacts = 0;
        }
    }
}

void RedBall::handleInput() {
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
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
    b2Vec2 vel = b2Body_GetLinearVelocity(playerBody);//获取物理对象的线速度
    float angVel = b2Body_GetAngularVelocity(playerBody);//获取物理对象的角速度

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right) && vel.x < 20)
    {
        b2Body_ApplyForceToCenter(playerBody, { 40.0f, 0.0f }, true);//应用一个力到物理对象的中心
        if (angVel < 15)
            b2Body_ApplyTorque(playerBody, 10, true);//应用一个扭矩到物理对象
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left) && vel.x > -20)
    {
        b2Body_ApplyForceToCenter(playerBody, { -40.0f, 0.0f }, true);
        if (angVel > -15)
            b2Body_ApplyTorque(playerBody, -10, true);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up) && footContacts > 0 && jumpCooldown == 0) {
        b2Body_ApplyForceToCenter(playerBody, { 0.0f, -1200.0f }, true);
        jumpCooldown = 15;
    }
    if (jumpCooldown > 0) jumpCooldown--;
}

void RedBall::drawScene() {
    window.clear(sf::Color::Black);
    if (debugDrawFlag) {
        b2World_Draw(worldId, &debugDraw);
    }
    if (groundShowFlag)
        window.draw(sBackground);

    window.draw(sGround);
    window.draw(sFlag);

    sPlatform.setPosition({ 100, 200 });
    window.draw(sPlatform);
    sPlatform.setPosition({ 550, 90 });
    window.draw(sPlatform);

    auto drawBody = [&](b2BodyId body, sf::Sprite& sprite) {
        b2Vec2 pos = b2Body_GetPosition(body);
        float angle = b2Rot_GetAngle(b2Body_GetRotation(body));
        sprite.setPosition({ pos.x * SCALE, pos.y * SCALE });
        sprite.setRotation(sf::degrees(angle * DEG));
        window.draw(sprite);
        };

    drawBody(boxBody1, sBox);
    drawBody(boxBody2, sBox);
    drawBody(boxBody3, sBox);
    drawBody(playerBody, sPlayer);
    drawBody(stoneBody, sStone);
    drawBody(swingBody, sSwing);

    window.display();
}
