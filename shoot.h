#ifndef __SHOOT_H__
#define __SHOOT_H__

#include "engine.h"
#include "collision.h"
#include <list>

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

    struct Bullet : public Point {
        Bullet(Sprite *sprite,float angle,float speed);
        ~Bullet();
        void move(float dt);
        Sprite *sprite;
        float vx,vy;
    };

    typedef std::list<Bullet*> Bullets;
    Bullets bullets;
};

#endif
