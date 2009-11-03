#include "../supershooter/shoot.h"
#include "except.h"
#include "utils.h"
#include <cmath>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

class StaticShip : public Ship {
public:
    StaticShip(Sprite *body,float health) : Ship(body,health,0) {}
    virtual bool move(float dt) { return true; }
};


class Logic : public Listener {
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_SPACE:
            for (float x=16; x<=SdlManager::get()->width-16; x+=16)
            for (float y=16; y<=SdlManager::get()->height-16; y+=16)
                BulletManager::get()->shoot(x,y,0,0,0,"bullet02");
            break;
        case SDLK_ESCAPE:
            return false; break;
        }
        return true;
    }
    virtual bool mouse_down(int button,float x,float y) { 
        switch (button) {
        case 1:
            aa->body->x=x; aa->body->y=y; break;
        case 3:
            BulletManager::get()->shoot(x,y,0,0,0,"bullet03"); break;
        case 4:
            aa->body->angle+=M_PI/180.*10.; break;
        case 5:
            aa->body->angle-=M_PI/180.*10.; break;
        default:
            cout<<button<<endl; break;
        };
        return true;
    };

    virtual bool frame_entered(float t,float dt) { return true; }
    virtual void register_self() {
        aa=new StaticShip(SpriteManager::get()->get_sprite("aa"),1000);
        aa->body->factorx=2.;
        aa->body->factory=.5;
        aa->body->angle=M_PI/180.*90.;
        aa->body->x=296;
        aa->body->y=296;
        ShipManager::get()->add_ship(aa,0);
    }

    StaticShip *aa;
};

int main() {
    try {
        SdlManager::init();

        std::string configfile;
        SpriteManager::init();
        configfile="config.xml"; if (not SpriteManager::get()->load_directory("data")) {
        configfile="../../config.xml"; if (not SpriteManager::get()->load_directory("../../data")) {
            cerr<<"can't locate sprite data..."<<endl;
            return 1;
        }}
        SpriteManager::get()->dump();

        CollisionManager::init();
        BulletManager::init();

        cout<<"using "<<configfile<<endl;
        ShipManager::init(2,configfile);

        SdlManager::get()->register_listener(BulletManager::get());
        SdlManager::get()->register_listener(ShipManager::get());
        SdlManager::get()->set_background_color(.5,.6,.7);
        {

            Logic logic;
            Fps fps;
            SdlManager::get()->register_listener(&logic);
            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->main_loop();
        }
        ShipManager::free();
        BulletManager::free();
        CollisionManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
        return 1;
    }
}


