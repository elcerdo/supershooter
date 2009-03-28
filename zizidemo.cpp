#include "engine.h"
#include "except.h"
#include "utils.h"
#include <list>
#include <iostream>
using std::cout;
using std::endl;

class Spawner : public Listener {
public:
    Spawner() {}
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
        Sprite *s=SpriteManager::get()->get_sprite("bullet00");
        s->x=x;
        s->y=y;
        sprites.push_back(s);

        AnimatedSprite *cb=dynamic_cast<AnimatedSprite*>(s->create_child("bullet00"));
        AnimatedSprite *ca=dynamic_cast<AnimatedSprite*>(s->create_child("burst00"));
        ca->repeat=false;
        ca->x=-16;
        ca->y=+16;
        cb->x=+16;
        cb->y=-16;
        cb->length=3;
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        for (Sprites::const_iterator i=sprites.begin(); i!=sprites.end(); i++) (*i)->draw(dt);
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

        {
        Spawner spawner;
        Fps fps;

        SdlManager::get()->register_listener(&fps);
        SdlManager::get()->register_listener(&spawner);
        SdlManager::get()->main_loop();
        }

        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


