#ifndef __SHOOT_H__
#define __SHOOT_H__

#include "engine.h"
#include "collision.h"
#include <vector>
#include <set>

//***********************************************************
struct Ship : public Area {
    Ship(float health);
    Ship(Sprite *body,float health);
    virtual ~Ship();
    virtual bool move(float dt)=0;
    virtual void draw(float dt) const;
    Sprite *body;
    float health;
};

class ShipManager : public Listener {
public:
    static ShipManager *get();
    static void free();
    static void init(size_t nspace=2);

    void add_ship(Ship *ship,size_t kspace);
protected:
    ShipManager(size_t nspace=2);
    ~ShipManager();

    virtual bool frame_entered(float t,float dt);
    virtual void unregister_self();

    typedef std::set<Ship*> Ships;
    typedef std::vector<Ships> Spaces;
    Spaces spaces;

    long int ncreated;
    long int ndestroyed;
};

//***********************************************************
struct Bullet : public Point {
    Bullet(Sprite *sprite,float angle,float speed,float damage);
    virtual ~Bullet();
    virtual void move(float dt);
    Sprite *sprite;
    float vx,vy;
    float damage;
};

class BulletManager : public Listener {
public:
    static BulletManager *get();
    static void free();
    static void init(size_t nspace=2);

    Bullet *shoot(float x,float y,float angle, float speed, size_t kspace, const std::string &name="bullet00",float damage=2.);
    Bullet *shoot_from_sprite(const Sprite *sprite,float rangle, float speed, size_t kspace, const std::string &name="bullet00",float damage=2.);
protected:
    BulletManager(size_t nspace);
    ~BulletManager();

    virtual bool frame_entered(float t,float dt);
    virtual void unregister_self();
    void move(float dt);
    void draw(float dt) const;

    typedef std::set<Bullet*> Bullets;
    typedef std::vector<Bullets> Spaces;
    Spaces spaces;

    long int ncreated;
    long int ndestroyed;
};

#endif
