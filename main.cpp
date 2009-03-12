#include "shoot.h"
#include "except.h"
#include <cmath>
#include <iostream>
using std::cout;
using std::endl;

class StaticShip : public Ship {
public:
    StaticShip(Sprite *body,float ix,float iy) : Ship(body,100) {
        *x=ix;
        *y=iy;
    }
    void move(float dt) {}
};

class StaticArea : public Area {
public:
    StaticArea(Sprite *sprite) : Area(&sprite->x,&sprite->y), sprite(sprite) { w=sprite->w; h=sprite->h; }
    ~StaticArea() { delete sprite; }

    void draw() const { sprite->draw(); }
protected:
    Sprite *sprite;
};


class BigShip : public Ship {
public:
    BigShip() : Ship(100), angle(0), speed(0), shooting(false), reload(0) {
        body=SpriteManager::get()->get_sprite("bigship00");
        body->z=-1;
        //body->factorx=2.;
        //body->factory=2.;
        turrel_left=dynamic_cast<AnimatedSprite*>(body->create_child("turret00"));
        turrel_left->x=-16;
        turrel_left->y=-8;
        turrel_left->cx=10;
        turrel_left->z=1;
        turrel_left->angle=M_PI/180.*15;
        turrel_right=dynamic_cast<AnimatedSprite*>(body->create_child("turret00"));
        turrel_right->x=-16;
        turrel_right->y=8;
        turrel_right->cx=10;
        turrel_right->z=1;
        turrel_right->angle=-M_PI/180.*15;

        body->x=100;
        body->y=100;
        
        x=&body->x;
        y=&body->y;
        w=100;
        h=100;
    };

    virtual void move(float dt) {
        *x+=dt*speed*cos(angle);
        *y+=dt*speed*sin(angle);
        body->angle=angle;

        if (reload>0) reload-=dt;

        if (shooting and reload<=0) {
            reload+=0.05;
            BulletManager::get()->shoot_from_sprite(turrel_left,0,300,0);
            BulletManager::get()->shoot_from_sprite(turrel_left,M_PI/180.*10.,300,0);
            BulletManager::get()->shoot_from_sprite(turrel_left,-M_PI/180.*10.,300,0);
            BulletManager::get()->shoot_from_sprite(turrel_right,0,300,0);
            BulletManager::get()->shoot_from_sprite(turrel_right,M_PI/180.*10.,300,0);
            BulletManager::get()->shoot_from_sprite(turrel_right,-M_PI/180.*10.,300,0);
        }

        if (shooting) { turrel_left->state=1; turrel_right->state=1; }
        else { turrel_left->state=0; turrel_right->state=0; }
    }

    bool shooting;
    float angle,speed;

    AnimatedSprite *turrel_left;
    AnimatedSprite *turrel_right;
protected:
    float reload;
};
    
class Spawner : public Listener {
public:
    Spawner() {
        test=new StaticArea(SpriteManager::get()->get_sprite("aa"));
        *test->x=200;
        *test->y=200;
        CollisionManager::get()->spaces[0].second.insert(test);
    }
    ~Spawner() {
        CollisionManager::get()->spaces[0].second.erase(test);
        delete test;
    }
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_SPACE:
            bigship->shooting=not bigship->shooting; break;
        case SDLK_ESCAPE:
            return false; break;
        }
        return true;
    }
    virtual bool key_up(SDLKey key) {
        switch (key) {
        case SDLK_SPACE:
            break;
        }
        return true;
    }

    virtual bool mouse_down(int button,float x,float y) {
        *test->x=x;
        *test->y=y;
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        const unsigned char *state=SdlManager::get()->get_key_state();
        if (state[SDLK_LEFT]) bigship->angle-=M_PI/180.*dt*180.;
        if (state[SDLK_RIGHT]) bigship->angle+=M_PI/180.*dt*180.;
        if (state[SDLK_UP]) bigship->speed+=dt*300.;
        if (state[SDLK_DOWN]) bigship->speed-=dt*300.;
        bigship->speed-=bigship->speed*1.*dt;

        float turrel_angle=M_PI/180.*(45.+45.*cos(2*M_PI*.4*t));
        bigship->turrel_left->angle=-turrel_angle;
        bigship->turrel_right->angle=turrel_angle;

        test->draw();

        return true;
    }

    virtual void register_self() {
        bigship=new BigShip;
        ShipManager::get()->add_ship(bigship,1);
        ShipManager::get()->add_ship(new StaticShip(SpriteManager::get()->get_sprite("font"),30,500),0);
    }

    BigShip *bigship;
    StaticArea *test;
};



int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        CollisionManager::init();
        BulletManager::init();
        ShipManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump();

        SdlManager::get()->register_listener(BulletManager::get());
        SdlManager::get()->register_listener(ShipManager::get());
        SdlManager::get()->set_background_color(.5,.6,.7);
        {
            Spawner spawner;
            SdlManager::get()->register_listener(&spawner);
            SdlManager::get()->main_loop();
        }
        ShipManager::free();
        BulletManager::free();
        CollisionManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
    }
}


