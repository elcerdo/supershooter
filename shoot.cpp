#include "shoot.h"

#include <cmath>
#include "collision.h"
#include "except.h"
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;

BulletManager::Bullet::Bullet(Sprite *sprite,float angle,float speed) : Point(&sprite->x,&sprite->y), sprite(sprite), vx(speed*cos(angle)), vy(speed*sin(angle)) {}
BulletManager::Bullet::~Bullet() { delete sprite; }
void BulletManager::Bullet::move(float dt) { *x+=dt*vx; *y+=dt*vy; }

//***********************************************************
static BulletManager *mBulletManager=NULL;

BulletManager *BulletManager::get() { return mBulletManager; }
void BulletManager::free() { if (mBulletManager) { delete mBulletManager; mBulletManager=NULL; } }
void BulletManager::init() {
    if (mBulletManager) throw Except(Except::SS_INIT_ERR);
    mBulletManager=new BulletManager();
}

BulletManager::BulletManager() {}

BulletManager::~BulletManager() { unregister_self(); }

void BulletManager::unregister_self() {
    for (Bullets::const_iterator i=bullets.begin(); i!=bullets.end(); i++) delete *i;
    bullets.clear();
}

bool BulletManager::frame_entered(float t,float dt) {
    move(dt);
    CollisionManager::get()->resolve_collision();
    CollisionManager::Space &space=CollisionManager::get()->spaces[0];

    Area::Points foo;
    for (CollisionManager::Areas::const_iterator i=space.second.begin(); i!=space.second.end(); i++) {
        const Area *area=*i;
        std::set_union(area->colliding.begin(),area->colliding.end(),foo.begin(),foo.end(),std::inserter(foo,foo.begin()));
    }

    for (Area::Points::const_iterator j=foo.begin(); j!=foo.end(); j++) {
       space.first.erase(*j);
       bullets.erase(dynamic_cast<Bullet*>(*j));
       delete *j; 
    }

    draw();
    return true;
}

void BulletManager::shoot(float x,float y,float angle, float speed,const std::string &name) {
    Sprite *sprite=SpriteManager::get()->get_sprite(name);
    sprite->x=x;
    sprite->y=y;
    Bullet *bullet=new Bullet(sprite,angle,speed);
    bullets.insert(bullet);
    CollisionManager::get()->spaces[0].first.insert(bullet);
}

void BulletManager::shoot_from_sprite(const Sprite *sprite,float rangle, float speed, const std::string &name) {
    float ax,ay,aangle,afactorx,afactory;
    sprite->absolute_coordinates(ax,ay,aangle,afactorx,afactory);
    shoot(ax,ay,aangle+rangle,speed,name);
}

void BulletManager::move(float dt) { 
    for (Bullets::iterator i=bullets.begin(); i!=bullets.end(); i++) {
        Bullet *bullet=*i;
        bullet->move(dt);
        if (bullet->sprite->x<-20 or bullet->sprite->x>SdlManager::get()->width+20 or bullet->sprite->y<-20 or bullet->sprite->y>SdlManager::get()->height+20) {
            delete bullet;
            bullets.erase(i);
            CollisionManager::get()->spaces[0].first.erase(bullet);
        }
    }
}

void BulletManager::draw() const { for (Bullets::const_iterator i=bullets.begin(); i!=bullets.end(); i++) (*i)->sprite->draw(); }

