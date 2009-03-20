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

class MainMenu : public Listener {
public:
    MainMenu() : state(IN_MENU), ship(NULL)  {
        flogo=SpriteManager::get()->get_sprite("fronttitle");
        blogo=SpriteManager::get()->get_sprite("backtitle");
        flogo->x=SdlManager::get()->width*.5;
        flogo->y=100;
        flogo->z=9;
        blogo->x=SdlManager::get()->width*.5;
        blogo->y=100;
        blogo->z=8.5;

        for (size_t k=0; k<255; k++) stars.insert(new Star(true));

    }
    ~MainMenu() {
        if (ship) delete ship;

        for (Stars::const_iterator i=stars.begin(); i!=stars.end(); i++) delete *i;

        delete flogo;
        delete blogo;
    }
protected:
    virtual bool key_down(SDLKey key) {
        if (key==SDLK_ESCAPE and state==IN_MENU) return false;
        if (key==SDLK_RETURN and state==IN_MENU) { state=IN_GAME; ShipManager::get()->flush_ships(); BulletManager::get()->flush_bullets(); ShipManager::get()->schedule_wave("mainwave"); ship=new BigShip; SdlManager::get()->register_listener(ship); }
        if (key==SDLK_ESCAPE and state==IN_GAME) { state=IN_MENU; SdlManager::get()->unregister_listener(ship); ShipManager::get()->flush_waves(); delete ship; ship=NULL; }
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        if (state==IN_MENU) {
            blogo->alpha=(.4+.2*cos(2*M_PI*t/4.))/2.;
            blogo->draw(dt);
            flogo->draw(dt);
        } else if (state==IN_GAME) {
            if (ship->health<0) { state=IN_MENU; SdlManager::get()->unregister_listener(ship); ShipManager::get()->flush_waves(); delete ship; }
        }


        for (Stars::const_iterator i=stars.begin(); i!=stars.end(); i++) {
            Star *current=*i;
            current->sprite->y+=current->v*dt;
            if (current->sprite->y>SdlManager::get()->height) {
                delete current;
                stars.erase(i);
                stars.insert(new Star);
            }
            current->sprite->draw(dt);
        }
        return true;
    }
    Sprite *flogo,*blogo;

    enum State {
        IN_MENU,
        IN_GAME,
    };
    State state;
    
    struct Star {
        Star(bool initial=false) : v(5.+100.*rand()/(RAND_MAX+1.)) {
            sprite=dynamic_cast<StateSprite*>(SpriteManager::get()->get_sprite("star"));
            sprite->state=rand()%sprite->nstate;
            sprite->y=-SdlManager::get()->height*rand()/(RAND_MAX+1.);
            if (initial) sprite->y*=-1;
            sprite->x=SdlManager::get()->width*rand()/(RAND_MAX+1.);
            sprite->z=-9.5;
            sprite->alpha=.5;
            sprite->factorx=.25+.75*rand()/(RAND_MAX+1.);
            sprite->factory=sprite->factorx;
        }
        ~Star() { delete sprite; }
        StateSprite *sprite;
        float v;
    };

    typedef std::set<Star*> Stars;
    Stars stars;
    BigShip *ship;
};




int main() {
    try {
        SdlManager::init();
        SdlManager::get()->set_background_color(0,0,0);

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

            //Killer killer;
            //BigShip bigship;
            //Pusher pusher;
            //SdlManager::get()->register_listener(&killer);
            //SdlManager::get()->register_listener(&pusher);
            //SdlManager::get()->register_listener(&bigship);

            Fps fps;
            MainMenu mainmenu;
            SdlManager::get()->register_listener(&mainmenu);
            SdlManager::get()->register_listener(&fps);
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


