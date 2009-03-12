#ifndef __SHOOT_H__
#define __SHOOT_H__

#include "engine.h"
#include "collision.h"
#include <vector>
#include <set>

class BulletManager : public Listener {
public:
    static BulletManager *get();
    static void free();
    static void init(size_t nspace=2);

    void shoot(float x,float y,float angle, float speed, size_t kspace=0, const std::string &name="bullet00");
    void shoot_from_sprite(const Sprite *sprite,float rangle, float speed, size_t kspace=0, const std::string &name="bullet00");
    void move(float dt);
    void draw() const;
protected:
    BulletManager(size_t nspace);
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

    typedef std::set<Bullet*> Bullets;
    typedef std::vector<Bullets> Spaces;
    Spaces spaces;

    long int ncreated;
    long int ndestroyed;
};

#endif
