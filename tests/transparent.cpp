#include "engine.h"
#include "except.h"
#include "utils.h"
#include "sound.h"
#include <iostream>
using std::cout;
using std::endl;

class Spawner : public Listener {
public:
    Spawner() {
        a = get_test_sprite(0);
        b = get_test_sprite(1);
        c = get_test_sprite(2);
        a->x = 100;
        a->y = 100;
        a->z = 2;
        b->x = 116;
        b->y = 116;
        b->z = 1;
        c->x = 132;
        c->y = 132;
        c->z = 0;
    }
    ~Spawner() {
        delete a;
        delete b;
        delete c;
    }
protected:
    StateSprite *get_test_sprite(int ss) {
        StateSprite *foo = dynamic_cast<StateSprite*>(SpriteManager::get()->get_sprite("test"));
        foo->state = ss;
        foo->factorx = 4;
        foo->factory = 4;
        return foo;
    }
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_ESCAPE:
            return false; break;
        }
        return true;
    }
    virtual bool mouse_down(int button,float x,float y) {
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        c->draw(dt);
        b->draw(dt);
        a->draw(dt);
        return true;
    }
    virtual void unregister_self() {
    }
    StateSprite *a;
    StateSprite *b;
    StateSprite *c;
};



int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        SoundManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->load_directory("../data");
        SpriteManager::get()->load_directory("../../data");
        SpriteManager::get()->dump(cout);

        {
        Spawner spawner;
        Fps fps;

        SdlManager::get()->register_listener(&fps);
        SdlManager::get()->register_listener(&spawner);
        SdlManager::get()->main_loop();
        }

        SoundManager::free();
        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


