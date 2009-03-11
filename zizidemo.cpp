#include "engine.h"
#include "except.h"
#include <list>
#include <iostream>
using std::cout;
using std::endl;

class Logger : public Listener {
public:
    Logger() : frame(0), update_t(0) {}
protected:
    virtual bool key_down(SDLKey key) { cout<<"key down"<<endl; return true; };
    virtual bool key_up(SDLKey key) { cout<<"key up"<<endl; return true; };
    virtual bool mouse_down(int button,float x,float y) { cout<<"mouse down"<<endl; return true; };
    virtual bool mouse_up(int button,float x,float y) {  cout<<"mouse up"<<endl; return true; };
    virtual bool frame_entered(float t,float dt) {
        frame++;
        if (t>update_t+5.) {
            cout<<static_cast<float>(frame)/(t-update_t)<<"fps"<<endl;
            frame=0;
            update_t=t;
        }
        return true;
    }

    virtual void register_self() { frame=0; cout<<"registered"<<endl; };
    virtual void unregister_self() { frame=0; cout<<"unregistered"<<endl; };
    int frame;
    float update_t;
};

class Spawner : public Listener {
public:
    ~Spawner() { unregister_self(); }
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_SPACE:
            cout<<"*****************************"<<endl;
            for (Sprites::const_iterator i=sprites.begin(); i!=sprites.end(); i++) (*i)->dump();
            break;
        case SDLK_ESCAPE:
            return false; break;
        }
        return true;
    }
    virtual bool mouse_down(int button,float x,float y) {
        Sprite *s=SpriteManager::get()->get_sprite("bullet");
        s->x=x;
        s->y=y;
        sprites.push_back(s);

        Sprite *ca=s->create_child("bullet");
        Sprite *cb=s->create_child("bullet");
        ca->x=-16;
        ca->y=+16;
        cb->x=+16;
        cb->y=-16;
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        for (Sprites::const_iterator i=sprites.begin(); i!=sprites.end(); i++) (*i)->draw();
        return true;
    }
    virtual void unregister_self() {
        while (not sprites.empty()) { delete sprites.back(); sprites.pop_back(); }
    }
    typedef std::list<Sprite*> Sprites;
    Sprites sprites;
};



int main() {
    try {
        SdlManager::init();
        SpriteManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump(cout);

        Spawner spawner;
        Logger logger;

        SdlManager::get()->register_listener(&logger);
        SdlManager::get()->register_listener(&spawner);
        SdlManager::get()->main_loop();

        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


