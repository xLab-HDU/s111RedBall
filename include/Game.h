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

class Game {
public:
    Game();
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
