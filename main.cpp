#include "shoot.h"
#include "except.h"
#include "utils.h"
#include "message.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
using std::cout;
using std::endl;

class BigShip : public Ship, public Listener {
public:
    BigShip() : Ship(100), shooting(false), reload(0) {
        life=SpriteManager::get()->get_text("life","font00");
        life->x=16;
        life->y=SdlManager::get()->height-16;

        body=SpriteManager::get()->get_sprite("bigship00");
        body->z=-1;
        //body->factorx=2.;
        //body->factory=2.;
        turrel_left=dynamic_cast<AnimatedSprite*>(body->create_child("turret00"));
        turrel_left->x=-16;
        turrel_left->y=-8;
        turrel_left->cx=10;
        turrel_left->z=1;
        turrel_left->angle=-M_PI/180.*15;
        turrel_right=dynamic_cast<AnimatedSprite*>(body->create_child("turret00"));
        turrel_right->x=-16;
        turrel_right->y=8;
        turrel_right->cx=10;
        turrel_right->z=1;
        turrel_right->angle=M_PI/180.*15;

        body->x=700;
        body->y=300;
        body->angle=-M_PI/2.;
    }
    ~BigShip() { delete life; }

    virtual bool move(float dt) {
        std::stringstream ss;
        ss<<std::fixed<<std::setprecision(0)<<health;
        life->update(ss.str());

        //body->x+=dt*speed*cos(angle);
        //body->y+=dt*speed*sin(angle);

        if (reload>0) reload-=dt;

        if (shooting and reload<=0) {
            reload+=0.05;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(turrel_left,0,600,0,"bullet00",10)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(turrel_left,M_PI/180.*10.,600,0,"bullet00",10)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(turrel_left,-M_PI/180.*10.,600,0,"bullet00",10)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(turrel_right,0,600,0,"bullet00",10)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(turrel_right,M_PI/180.*10.,600,0,"bullet00",10)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(turrel_right,-M_PI/180.*10.,600,0,"bullet00",10)->sprite)->speed=20.;
        }

        if (shooting) { turrel_left->state=1; turrel_right->state=1; }
        else { turrel_left->state=0; turrel_right->state=0; }

        return true;
    }
    virtual void draw(float dt) const {
        body->draw(dt);
        life->draw(dt);
    }


protected:
    //virtual bool key_down(SDLKey key) {
    //    switch (key) {
    //    case SDLK_SPACE:
    //        shooting=not shooting; break;
    //    }
    //    return true;
    //}

    virtual bool mouse_down(int button, float x,float y) {
        if (button==1) shooting=true;
        else if (button==4) { turrel_left->angle+=M_PI/180.*5; turrel_right->angle-=M_PI/180.*5.; }
        else if (button==5) { turrel_left->angle-=M_PI/180.*5; turrel_right->angle+=M_PI/180.*5.; }
        return true;
    }
    virtual bool mouse_up(int button,float x,float y) {
        if (button==1) shooting=false;
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        //const unsigned char *state=SdlManager::get()->get_key_state();
        //if (state[SDLK_LEFT]) angle-=M_PI/180.*dt*180.;
        //if (state[SDLK_RIGHT]) angle+=M_PI/180.*dt*180.;
        //if (state[SDLK_UP]) speed+=dt*300.;
        //if (state[SDLK_DOWN]) speed-=dt*300.;
        //speed-=speed*1.*dt;
        SdlManager::get()->get_mouse_position(body->x,body->y);


        //float turrel_angle=M_PI/180.*(30.+20.*cos(2*M_PI*.8*t));
        //turrel_left->angle=-turrel_angle;
        //turrel_right->angle=turrel_angle;

        move(dt);
        draw(dt);

        if (health<0) SdlManager::get()->unregister_listener(this);

        return true;
    }

    virtual void register_self() {
        CollisionManager::get()->spaces[1].second.insert(this);
    }

    virtual void unregister_self() {
        CollisionManager::get()->spaces[1].second.erase(this);
    }

    AnimatedSprite *turrel_left;
    AnimatedSprite *turrel_right;
    Text *life;
    bool shooting;
    //float angle,speed;
    float reload;
};
    

class Pusher : public Listener {
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_SPACE:
            ShipManager::get()->schedule_wave("mainwave"); break;
        }
        return true;
    }
    virtual bool frame_entered(float t,float dt) { return true; }
};


int main() {
    try {
        SdlManager::init();
        SdlManager::get()->set_background_color(.5,.6,.7);

        SpriteManager::init();
        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump();

        CollisionManager::init();

        MessageManager::init();
        SdlManager::get()->register_listener(MessageManager::get());

        BulletManager::init();
        SdlManager::get()->register_listener(BulletManager::get());

        ShipManager::init();
        SdlManager::get()->register_listener(ShipManager::get());
        ShipManager::get()->dump();
        {
            //ShipManager::get()->launch_enemy_ship("bigship","main",400,0,M_PI/2.);
            //ShipManager::get()->launch_enemy_ship("bigship","main",600,0,M_PI/2.);
            //XmlShip *aa;
            //for (float y=200; y<=800; y+=50) ShipManager::get()->launch_enemy_ship("basicship","left",y,-100,M_PI/2.);
            //for (float y=200; y<=800; y+=50) ShipManager::get()->launch_enemy_ship("basicship","right",y,-50,M_PI/2.);

            Killer killer;
            BigShip bigship;
            Fps fps;
            Pusher pusher;
            SdlManager::get()->register_listener(&killer);
            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->register_listener(&pusher);
            SdlManager::get()->register_listener(&bigship);
            SdlManager::get()->main_loop();
        }
        ShipManager::free();
        BulletManager::free();
        MessageManager::free();
        CollisionManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
        return 1;
    }
}


