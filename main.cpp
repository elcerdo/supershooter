#include "engine.h"
#include "except.h"
#include <list>
#include <iostream>
using std::cout;
using std::endl;

class Logger : public Listener {
public:
    Logger() : frame(0), update_ticks(0) {}
    virtual bool key_down(SDLKey key) { cout<<"key down"<<endl; return true; };
    virtual bool key_up(SDLKey key) { cout<<"key up"<<endl; return true; };
    virtual bool mouse_down(Uint8 button,float x,float y) { cout<<"mouse down"<<endl; return true; };
    virtual bool mouse_up(Uint8 button,float x,float y) {  cout<<"mouse up"<<endl; return true; };
    virtual bool frame_entered(Uint32 ticks) {
        frame++;
        if (ticks>update_ticks+5000) {
            cout<<1000.*static_cast<float>(frame)/(ticks-update_ticks)<<"fps"<<endl;
            frame=0;
            update_ticks=ticks;
        }
        return true;
    }

    virtual void register_self() { frame=0; cout<<"registered"<<endl; };
    virtual void unregister_self() { frame=0; cout<<"unregistered"<<endl; };
protected:
    int frame;
    Uint32 update_ticks;
};

class Spawner : public Listener {
public:
    virtual bool mouse_down(Uint8 button,float x,float y) {
        Sprite *s=SpriteManager::get()->get_sprite("bullet");
        s->x=x;
        s->y=y;
        sprites.push_back(s);
        return true;
    }
    virtual bool frame_entered(Uint32 ticks) {
        for (Sprites::const_iterator i=sprites.begin(); i!=sprites.end(); i++) (*i)->draw();
        return true;
    }
    virtual void unregisted() {
        while (not sprites.empty()) { delete sprites.back(); sprites.pop_back(); }
    }
protected:
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

        SdlManager::get()->unregister_listener(&logger);
        SdlManager::get()->main_loop();

        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


