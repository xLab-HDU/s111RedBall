#pragma once

#include "DebugDrawUtils.h"

class RedBall {
public:
    RedBall();
    void run();

    void loadTextures();
    void setWall(int x, int y, int w, int h, b2WorldId worldId);
    void createSprites();
    void createWorld();
    void createObjects();

    void handleInput();
    void contactDetect();
    void drawScene();

    sf::RenderWindow window;
    sf::ContextSettings contextSettings;
    b2WorldId worldId;

    sf::Texture textures[7];
    sf::Sprite sGround, sPlayer, sBox, sStone, sSwing, sPlatform, sFlag, sBackground;

    b2BodyId body0, playerBody, stoneBody, swingBody;
    b2BodyId boxBody1, boxBody2, boxBody3;

    int footContacts = 0;
    int jumpCooldown = 0;
    bool groundShowFlag = false;
    bool debugDrawFlag = true;

    b2DebugDraw debugDraw{};
    DrawContext ctx;
};
