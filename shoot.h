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
class BulletManager : public Listener {
public:
    static BulletManager *get();
    static void free();
    static void init();

    void shoot(float x,float y,float angle, float speed, const std::string &name="bullet");
    void shoot_from_sprite(const Sprite *sprite,float rangle, float speed, const std::string &name="bullet");
    void move(float dt);
    void draw() const;
protected:
    BulletManager();
    ~BulletManager();

    virtual bool frame_entered(float t,float dt);
    virtual void unregister_self();

    typedef std::list<Bullet> Bullets;
    Bullets bullets;
};

#endif
