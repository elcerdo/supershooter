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
void BulletManager::init(size_t nspace) {
    if (mBulletManager) throw Except(Except::SS_INIT_ERR);
    mBulletManager=new BulletManager(nspace);
}

BulletManager::BulletManager(size_t nspace) : spaces(nspace), ncreated(0), ndestroyed(0) {}
BulletManager::~BulletManager() {
    unregister_self();
    cout<<ncreated<<" bullets created, "<<ndestroyed<<" bullets destroyed"<<endl;
    if (ncreated==ndestroyed) cout<<"no leak detected"<<endl;
    else cout<<"leak detected"<<endl;
}

void BulletManager::unregister_self() {
    for (Spaces::iterator bullets=spaces.begin(); bullets!=spaces.end(); bullets++) {
        for (Bullets::const_iterator i=bullets->begin(); i!=bullets->end(); i++) delete *i;
        ndestroyed+=bullets->size();
        bullets->clear();
    }
}

bool BulletManager::frame_entered(float t,float dt) {
    move(dt);

    CollisionManager::get()->resolve_collision();
    for (size_t k=0; k<spaces.size(); k++) {
        CollisionManager::Space &space=CollisionManager::get()->spaces[k];
        Bullets &bullets=spaces[k];

        Area::Points foo;
        for (CollisionManager::Areas::const_iterator i=space.second.begin(); i!=space.second.end(); i++) {
            const Area *area=*i;
            std::set_union(area->colliding.begin(),area->colliding.end(),foo.begin(),foo.end(),std::inserter(foo,foo.begin()));
        }

        for (Area::Points::const_iterator j=foo.begin(); j!=foo.end(); j++) {
           space.first.erase(*j);
           bullets.erase(dynamic_cast<Bullet*>(*j));
           delete *j; 
           ndestroyed++;
        }
    }

    draw();
    return true;
}

void BulletManager::shoot(float x,float y,float angle,float speed,size_t kspace,const std::string &name) {
    Sprite *sprite=SpriteManager::get()->get_sprite(name);
    sprite->x=x;
    sprite->y=y;
    Bullet *bullet=new Bullet(sprite,angle,speed);
    spaces[kspace].insert(bullet);
    CollisionManager::get()->spaces[kspace].first.insert(bullet);

    ncreated++;
}

void BulletManager::shoot_from_sprite(const Sprite *sprite,float rangle,float speed,size_t kspace,const std::string &name) {
    float ax,ay,aangle,afactorx,afactory;
    sprite->absolute_coordinates(ax,ay,aangle,afactorx,afactory);
    shoot(ax,ay,aangle+rangle,speed,kspace,name);
}

void BulletManager::move(float dt) { 
    size_t kspace=0;
    for (Spaces::iterator j=spaces.begin(); j!=spaces.end(); j++) {
        Bullets &bullets=*j;
        for (Bullets::iterator i=bullets.begin(); i!=bullets.end(); i++) {
            Bullet *bullet=*i;
            bullet->move(dt);
            if (bullet->sprite->x<-20 or bullet->sprite->x>SdlManager::get()->width+20 or bullet->sprite->y<-20 or bullet->sprite->y>SdlManager::get()->height+20) {
                delete bullet;
                bullets.erase(i);
                CollisionManager::get()->spaces[kspace].first.erase(bullet);
                ndestroyed++;
            }
        }
        kspace++;
    }
}

void BulletManager::draw() const {
    for (Spaces::const_iterator bullets=spaces.begin(); bullets!=spaces.end(); bullets++)
    for (Bullets::const_iterator i=bullets->begin(); i!=bullets->end(); i++)
        (*i)->sprite->draw();
}

