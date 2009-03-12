#include "shoot.h"

#include <cmath>
#include "collision.h"
#include "except.h"

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
    while (not bullets.empty()) { delete bullets.back(); bullets.pop_back(); }
}

bool BulletManager::frame_entered(float t,float dt) {
    move(dt);
    draw();
    return true;
}

void BulletManager::shoot(float x,float y,float angle, float speed,const std::string &name) {
    Sprite *sprite=SpriteManager::get()->get_sprite(name);
    sprite->x=x;
    sprite->y=y;
    Bullet *bullet=new Bullet(sprite,angle,speed);
    bullets.push_back(bullet);
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
            i=bullets.erase(i);
            CollisionManager::get()->spaces[0].first.erase(bullet);
        }
    }
}

void BulletManager::draw() const { for (Bullets::const_iterator i=bullets.begin(); i!=bullets.end(); i++) (*i)->sprite->draw(); }

