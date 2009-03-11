#ifndef __SHOOT_H__
#define __SHOOT_H__

#include "engine.h"
#include <list>

struct Bullet {
    Bullet(Sprite *sprite,float angle,float speed);
    void move(float dt);
    Sprite *sprite;
    float vx,vy;
};

//***********************************************************
class BulletManager {
public:
    static BulletManager *get();
    static void free();
    static void init();

    void shoot(float x,float y,float angle, float speed);
    void shoot_from_sprite(const Sprite *sprite,float rangle, float speed);
    void move(float dt);
    void draw() const;
protected:
    BulletManager();
    ~BulletManager();

    typedef std::list<Bullet> Bullets;
    Bullets bullets;
};

#endif
