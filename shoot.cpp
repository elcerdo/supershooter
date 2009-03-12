#include "shoot.h"

#include <cmath>
#include "collision.h"
#include "except.h"
#include <iostream>
#include <algorithm>
#include <typeinfo>
using std::cout;
using std::endl;

//***********************************************************
Ship::Ship(float health) : body(NULL), health(health) {}
Ship::Ship(Sprite *body,float health) : Area(&body->x,&body->y), body(body), health(health) { w=body->w;  h=body->h; }
Ship::~Ship() { if (body) delete body; }
void Ship::draw() const { if (body) body->draw(); }

static ShipManager *mShipManager=NULL;

ShipManager *ShipManager::get() { return mShipManager; }
void ShipManager::free() { if (mShipManager) { delete mShipManager; mShipManager=NULL; } }
void ShipManager::init(size_t nspace) {
    if (mShipManager) throw Except(Except::SS_INIT_ERR);
    mShipManager=new ShipManager(nspace);
}

ShipManager::ShipManager(size_t nspace) : spaces(nspace), ncreated(0), ndestroyed(0) {}
ShipManager::~ShipManager() {
    unregister_self();
    cout<<ncreated<<" ships created, "<<ndestroyed<<" ships destroyed: ";
    if (ncreated==ndestroyed) cout<<"no leak detected"<<endl;
    else cout<<"leak detected"<<endl;
}

void ShipManager::unregister_self() {
    size_t kspace=0;
    for (Spaces::iterator ships=spaces.begin(); ships!=spaces.end(); ships++) {
        for (Ships::const_iterator i=ships->begin(); i!=ships->end(); i++) {
            CollisionManager::get()->spaces[kspace].second.erase(*i);
            delete *i;
        }
        ndestroyed+=ships->size();
        ships->clear();
        kspace++;
    }
}

bool ShipManager::frame_entered(float t,float dt) {

    size_t kspace=0;
    for (Spaces::iterator ships=spaces.begin(); ships!=spaces.end(); ships++) {
        for (Ships::iterator i=ships->begin(); i!=ships->end(); i++) {
            if ((*i)->health<0) {
                delete *i;
                CollisionManager::get()->spaces[kspace].second.erase(*i);
                ships->erase(i);
                ndestroyed++;
                continue;
            }
            (*i)->move(dt);
            (*i)->draw();
        }
        kspace++;
    }

    return true;
}

void ShipManager::add_ship(Ship *ship,size_t kspace) {
    spaces[kspace].insert(ship);
    CollisionManager::get()->spaces[kspace].second.insert(ship);
    
    ncreated++;
}

//***********************************************************
Bullet::Bullet(Sprite *sprite,float angle,float speed,float damage) : Point(&sprite->x,&sprite->y), sprite(sprite), vx(speed*cos(angle)), vy(speed*sin(angle)), damage(damage) {}
Bullet::~Bullet() { delete sprite; }
void Bullet::move(float dt) { *x+=dt*vx; *y+=dt*vy; }

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
    cout<<ncreated<<" bullets created, "<<ndestroyed<<" bullets destroyed: ";
    if (ncreated==ndestroyed) cout<<"no leak detected"<<endl;
    else cout<<"leak detected"<<endl;
}

void BulletManager::unregister_self() {
    size_t kspace=0;
    for (Spaces::iterator bullets=spaces.begin(); bullets!=spaces.end(); bullets++) {
        for (Bullets::const_iterator i=bullets->begin(); i!=bullets->end(); i++) {
            CollisionManager::get()->spaces[kspace].first.erase(*i);
            delete *i;
        }
        ndestroyed+=bullets->size();
        bullets->clear();
        kspace++;
    }
}

bool BulletManager::frame_entered(float t,float dt) {
    move(dt);

    CollisionManager::get()->resolve_collision();
    //CollisionManager::get()->dump();
    for (size_t k=0; k<spaces.size(); k++) {
        CollisionManager::Space &space=CollisionManager::get()->spaces[k];
        Bullets &bullets=spaces[k];

        Area::Points foo;
        for (CollisionManager::Areas::const_iterator i=space.second.begin(); i!=space.second.end(); i++) {
            const Area *area=*i;
            std::set_union(area->colliding.begin(),area->colliding.end(),foo.begin(),foo.end(),std::inserter(foo,foo.begin()));
            //cout<<(dynamic_cast<const Ship*>(area)!=NULL)<<" "<<typeid(*area).before(typeid(Ship))<<endl;
            if (const Ship *ship=dynamic_cast<const Ship*>(area)) { //area is a ship so deal the damage //FIXME use of typeid is time constant
                for (Area::Points::const_iterator j=ship->colliding.begin(); j!=ship->colliding.end(); j++) {
                    if (const Bullet *bullet=dynamic_cast<const Bullet*>(*j)) {
                        const_cast<Ship*>(ship)->health-=bullet->damage;
                    }
                }
            }


        }
        //cout<<"---------"<<endl;

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

void BulletManager::shoot(float x,float y,float angle,float speed,size_t kspace,const std::string &name,float damage) {
    Sprite *sprite=SpriteManager::get()->get_sprite(name);
    sprite->x=x;
    sprite->y=y;
    Bullet *bullet=new Bullet(sprite,angle,speed,damage);
    spaces[kspace].insert(bullet);
    CollisionManager::get()->spaces[kspace].first.insert(bullet);

    ncreated++;
}

void BulletManager::shoot_from_sprite(const Sprite *sprite,float rangle,float speed,size_t kspace,const std::string &name,float damage) {
    float ax,ay,aangle,afactorx,afactory;
    sprite->absolute_coordinates(ax,ay,aangle,afactorx,afactory);
    shoot(ax,ay,aangle+rangle,speed,kspace,name,damage);
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

