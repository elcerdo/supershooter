#include "shoot.h"
#include "except.h"
#include <cmath>

class BigShip {
public:
    BigShip() : tx(300), ty(300), angle(0), speed(0), shooting(false), reload(0) {
        body=SpriteManager::get()->get_sprite("bigship01");
        body->z=-1;
        body->factorx=2.;
        body->factory=2.;
        turrel_left=dynamic_cast<AnimatedSprite*>(body->create_child("turrel"));
        turrel_left->x=-16;
        turrel_left->y=-8;
        turrel_left->cx=10;
        turrel_left->z=1;
        turrel_left->angle=M_PI/180.*15;
        turrel_right=dynamic_cast<AnimatedSprite*>(body->create_child("turrel"));
        turrel_right->x=-16;
        turrel_right->y=8;
        turrel_right->cx=10;
        turrel_right->z=1;
        turrel_right->angle=-M_PI/180.*15;

        body->x=100;
        body->y=100;
    };
    ~BigShip() {
        delete body;
    }

    void move(float dt) {
        body->x+=dt*speed*cos(angle);
        body->y+=dt*speed*sin(angle);
        body->angle=angle;

        if (reload>0) reload-=dt;

        if (shooting and reload<=0) {
            reload+=0.15;
            BulletManager::get()->shoot_from_sprite(turrel_left,0,300);
            BulletManager::get()->shoot_from_sprite(turrel_left,M_PI/180.*10.,300);
            BulletManager::get()->shoot_from_sprite(turrel_left,-M_PI/180.*10.,300);
            BulletManager::get()->shoot_from_sprite(turrel_right,0,300);
            BulletManager::get()->shoot_from_sprite(turrel_right,M_PI/180.*10.,300);
            BulletManager::get()->shoot_from_sprite(turrel_right,-M_PI/180.*10.,300);
        }

        if (shooting) { turrel_left->state=1; turrel_right->state=1; }
        else { turrel_left->state=0; turrel_right->state=0; }
    }

    void draw() const { body->draw(); }

    bool shooting;
    float angle,speed;
    float tx,ty;

    AnimatedSprite *turrel_left;
    AnimatedSprite *turrel_right;
protected:
    float reload;
    Sprite *body;
};
    
class Spawner : public Listener {
public:
    Spawner() {}
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

    virtual bool mouse_down(int button,float x,float y) {
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        const unsigned char *state=SdlManager::get()->get_key_state();
        if (state[SDLK_LEFT]) bigship.angle-=M_PI/180.*dt*180.;
        if (state[SDLK_RIGHT]) bigship.angle+=M_PI/180.*dt*180.;
        if (state[SDLK_UP]) bigship.speed+=dt*300.;
        if (state[SDLK_DOWN]) bigship.speed-=dt*300.;
        bigship.speed-=bigship.speed*1.*dt;

        float turrel_angle=M_PI/180.*(45.+45.*cos(2*M_PI*.4*t));
        bigship.turrel_left->angle=-turrel_angle;
        bigship.turrel_right->angle=turrel_angle;

        bigship.move(dt);
        bigship.draw();

        return true;
    }
    BigShip bigship;
};



int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        BulletManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump();

        Spawner spawner;


        SdlManager::get()->register_listener(&spawner);
        SdlManager::get()->register_listener(BulletManager::get());
        SdlManager::get()->main_loop();

        SdlManager::free();
        SpriteManager::free();
        BulletManager::free();
    } catch (Except e) {
        e.dump();
    }
}


