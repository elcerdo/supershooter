#include "engine.h"
#include "except.h"
#include "utils.h"
#include "sound.h"
#include <list>
#include <iostream>
using std::cout;
using std::endl;

class Spawner : public Listener {
public:
    Spawner() {
        cursor=SpriteManager::get()->get_sprite("cursor");
        cursor->cx=cursor->w/2;
        cursor->cy=cursor->h/2;
        cursor->z=3;
        click=SoundManager::get()->get_sfx("click");
    }
    ~Spawner() { unregister_self(); delete cursor; delete click; }
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_m:
            SoundManager::get()->toggle_musics();
            break;
        case SDLK_s:
            SoundManager::get()->toggle_sfxs();
            break;
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

        click->play_once();
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        SdlManager::get()->get_mouse_position(cursor->x,cursor->y);
        cursor->draw(dt);
        for (Sprites::const_iterator i=sprites.begin(); i!=sprites.end(); i++) (*i)->draw(dt);
        return true;
    }
    virtual void unregister_self() {
        while (not sprites.empty()) { delete sprites.back(); sprites.pop_back(); }
    }
    typedef std::list<Sprite*> Sprites;
    Sprite *cursor;
    Sfx *click;
    Sprites sprites;
};



int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        SoundManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->load_directory("../../data");
        SpriteManager::get()->dump(cout);

        SoundManager::get()->load_directory("data");
        SoundManager::get()->load_directory("../../data");
        SoundManager::get()->play_musics_continuious=true;
        SoundManager::get()->play_music("ultraetron");
        SoundManager::get()->dump(cout);

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


