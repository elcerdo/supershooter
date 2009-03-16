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
Ship::Ship(Sprite *body,float health) : body(body), health(health) {}
Ship::~Ship() { delete body; }
void Ship::draw(float dt) const { body->draw(dt); }
float Ship::get_x() const { return body->x; }
float Ship::get_y() const { return body->y; }
float Ship::get_left() const { 
    float dx0=(body->w*body->factorx*cos(body->angle)+body->h*body->factory*sin(body->angle))/2.;
    float dx1=(body->w*body->factorx*cos(body->angle)-body->h*body->factory*sin(body->angle))/2.;
    if (dx0<0) dx0=-dx0;
    if (dx1<0) dx1=-dx1;
    return dx0<dx1 ? body->x-dx1 : body->x-dx0;
}
float Ship::get_right() const { 
    float dx0=(body->w*body->factorx*cos(body->angle)-body->h*body->factory*sin(body->angle))/2.;
    float dx1=(body->w*body->factorx*cos(body->angle)+body->h*body->factory*sin(body->angle))/2.;
    if (dx0<0) dx0=-dx0;
    if (dx1<0) dx1=-dx1;
    return dx0<dx1 ? body->x+dx1 : body->x+dx0;
}
float Ship::get_top() const {
    float dy0=(body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    float dy1=(-body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    if (dy0<0) dy0=-dy0;
    if (dy1<0) dy1=-dy1;
    return dy0<dy1 ? body->y-dy1 : body->y-dy0;
}
float Ship::get_bottom() const {
    float dy0=(body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    float dy1=(-body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    if (dy0<0) dy0=-dy0;
    if (dy1<0) dy1=-dy1;
    return dy0<dy1 ? body->y+dy1 : body->y+dy0;
}
bool Ship::collide_with(const Point *point) const {
    float dx=(point->get_x()-body->x);
    float dy=(point->get_y()-body->y);
    float lx=(dx*cos(body->angle)+dy*sin(body->angle))*2./body->factorx;
    if (lx<0) lx=-lx;
    if (lx>body->w) return false;
    float ly=(-dx*sin(body->angle)+dy*cos(body->angle))*2./body->factory;
    if (ly<0) ly=-ly;
    if (ly>body->h) return false;
    return true;
}

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
            if ((*i)->health<0 or not (*i)->move(dt)) {
                delete *i;
                CollisionManager::get()->spaces[kspace].second.erase(*i);
                ships->erase(i);
                ndestroyed++;
                continue;
            }
            (*i)->draw(dt);
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
Bullet::Bullet(Sprite *sprite,float angle,float speed,float damage) : sprite(sprite), vx(speed*cos(angle)), vy(speed*sin(angle)), damage(damage) { sprite->angle=angle;}
Bullet::~Bullet() { delete sprite; }
float Bullet::get_x() const { return sprite->x; }
float Bullet::get_y() const { return sprite->y; }
void Bullet::move(float dt) { sprite->x+=dt*vx; sprite->y+=dt*vy; }

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

    draw(dt);
    return true;
}

Bullet *BulletManager::shoot(float x,float y,float angle,float speed,size_t kspace,const std::string &name,float damage) {
    Sprite *sprite=SpriteManager::get()->get_sprite(name);
    sprite->x=x;
    sprite->y=y;
    Bullet *bullet=new Bullet(sprite,angle,speed,damage);
    spaces[kspace].insert(bullet);
    CollisionManager::get()->spaces[kspace].first.insert(bullet);
    ncreated++;
    return bullet;
}

Bullet *BulletManager::shoot_from_sprite(const Sprite *sprite,float rangle,float speed,size_t kspace,const std::string &name,float damage) {
    float ax,ay,aangle,afactorx,afactory;
    sprite->absolute_coordinates(ax,ay,aangle,afactorx,afactory);
    return shoot(ax,ay,aangle+rangle,speed,kspace,name,damage);
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

void BulletManager::draw(float dt) const {
    for (Spaces::const_iterator bullets=spaces.begin(); bullets!=spaces.end(); bullets++)
    for (Bullets::const_iterator i=bullets->begin(); i!=bullets->end(); i++)
        (*i)->sprite->draw(dt);
}

