#include "shoot.h"

#include <cmath>
#include "except.h"

Bullet::Bullet(Sprite *sprite,float angle,float speed) : sprite(sprite), vx(speed*cos(angle)), vy(speed*sin(angle)) {}
void Bullet::move(float dt) { sprite->x+=dt*vx; sprite->y+=dt*vy; }


//***********************************************************
static BulletManager *mBulletManager=NULL;

BulletManager *BulletManager::get() { return mBulletManager; }
void BulletManager::free() { if (mBulletManager) { delete mBulletManager; mBulletManager=NULL; } }
void BulletManager::init() {
    if (mBulletManager) throw Except(Except::SS_INIT_ERR);
    mBulletManager=new BulletManager();
}

BulletManager::BulletManager() {}

BulletManager::~BulletManager() {
    while (not bullets.empty()) { delete bullets.back().sprite; bullets.pop_back(); }
}

void BulletManager::shoot(float x,float y,float angle, float speed) {
    Sprite *sprite=SpriteManager::get()->get_sprite("bullet");
    sprite->x=x;
    sprite->y=y;
    bullets.push_back(Bullet(sprite,angle,speed));
}

void BulletManager::shoot_from_sprite(const Sprite *sprite,float rangle, float speed) {
    float ax,ay,aangle;
    sprite->absolute_coordinates(ax,ay,aangle);
    shoot(ax,ay,aangle+rangle,speed);
}

void BulletManager::move(float dt) { 
    for (Bullets::iterator i=bullets.begin(); i!=bullets.end(); i++) {
        i->move(dt);
        if (i->sprite->x<-20 or i->sprite->x>SdlManager::get()->width+20 or i->sprite->y<-20 or i->sprite->y>SdlManager::get()->height+20) {
            delete i->sprite;
            i=bullets.erase(i);
        }
    }
}

void BulletManager::draw() const { for (Bullets::const_iterator i=bullets.begin(); i!=bullets.end(); i++) i->sprite->draw(); }

