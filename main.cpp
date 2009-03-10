#include "shoot.h"
#include "except.h"
#include <iostream>
#include <cmath>
using std::cout;
using std::endl;

class BigShip {
public:
    BigShip() : tx(300), ty(300), angle(0), speed(0), shooting(false), reload(0) {
        body=SpriteManager::get()->get_sprite("bigship01");
        turrel_left=dynamic_cast<AnimatedSprite*>(SpriteManager::get()->get_sprite("turrel"));
        turrel_right=dynamic_cast<AnimatedSprite*>(SpriteManager::get()->get_sprite("turrel"));

        body->x=100;
        body->y=100;
    };
    ~BigShip() {
        delete body;
        delete turrel_left;
        delete turrel_right;
    }

    void move(float dt) {
        body->x+=dt*speed*cos(angle);
        body->y+=dt*speed*sin(angle);
        body->angle=angle;

        turrel_left->x=body->x+8*sin(body->angle);
        turrel_left->y=body->y-8*cos(body->angle);
        turrel_left->angle=angle;
        turrel_right->x=body->x-8*sin(body->angle);
        turrel_right->y=body->y+8*cos(body->angle);
        turrel_right->angle=angle;

        if (reload>0) reload-=dt;

        if (shooting and reload<=0) {
            reload+=0.5;
            BulletManager::get()->shoot(turrel_left->x,turrel_left->y,turrel_left->angle,300);
            BulletManager::get()->shoot(turrel_left->x,turrel_left->y,turrel_left->angle-M_PI/180.*15.,300);
            BulletManager::get()->shoot(turrel_left->x,turrel_left->y,turrel_left->angle+M_PI/180.*15.,300);
            BulletManager::get()->shoot(turrel_right->x,turrel_right->y,turrel_right->angle,300);
            BulletManager::get()->shoot(turrel_right->x,turrel_right->y,turrel_right->angle-M_PI/180.*15.,300);
            BulletManager::get()->shoot(turrel_right->x,turrel_right->y,turrel_right->angle+M_PI/180.*15.,300);
        }

        if (shooting) { turrel_left->state=1; turrel_right->state=1; }
        else { turrel_left->state=0; turrel_right->state=0; }
    }

    void draw() const { body->draw(); turrel_left->draw(); turrel_right->draw(); }

    bool shooting;
    float angle,speed;
    float tx,ty;
protected:
    float reload;
    Sprite *body;
    AnimatedSprite *turrel_left;
    AnimatedSprite *turrel_right;
};
    
class Spawner : public Listener {
public:
    Spawner() : old_ticks(0) {}
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_SPACE:
            bigship.shooting=not bigship.shooting; break;
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

    virtual bool mouse_down(Uint8 button,float x,float y) {
        return true;
    }
    virtual bool frame_entered(Uint32 ticks) {
        float dt=(ticks-old_ticks)/1000.;
        old_ticks=ticks;

        Uint8 *state=SDL_GetKeyState(NULL);
        if (state[SDLK_LEFT]) bigship.angle-=M_PI/180.*dt*180.;
        if (state[SDLK_RIGHT]) bigship.angle+=M_PI/180.*dt*180.;
        if (state[SDLK_UP]) bigship.speed+=dt*300.;
        if (state[SDLK_DOWN]) bigship.speed-=dt*300.;
        bigship.speed-=bigship.speed*1.*dt;
        bigship.move(dt);
        bigship.draw();

        BulletManager::get()->move(dt);
        BulletManager::get()->draw();

        return true;
    }
    Uint32 old_ticks;
    BigShip bigship;
};



int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        BulletManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump(std::cout);

        Spawner spawner;
        SdlManager::get()->register_listener(&spawner);
        SdlManager::get()->main_loop();

        SdlManager::free();
        SpriteManager::free();
        BulletManager::free();
    } catch (Except e) {
        e.dump();
    }
}


