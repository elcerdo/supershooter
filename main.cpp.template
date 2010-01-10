#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
using std::cout;
using std::cerr;
using std::endl;

class MainApp : public Listener {
public:
    MainApp() {
    }
    ~MainApp() {
        unregister_self();
    }
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_f:
            SdlManager::get()->toogle_fullscreen();
            break;
        case SDLK_ESCAPE:
            return false;
            break;
        default:
            break;
        }
        return true;
    }
    virtual bool mouse_down(int button,float x,float y) {
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        return true;
    }
    virtual void unregister_self() {
    }
};

int main() {
    try {
        srand(time(NULL));

        SdlManager::init();
        SdlManager::get()->set_background_color(0,0,0);

        SoundManager::init();
        if (not SoundManager::get()->load_directory("data"))
        if (not SoundManager::get()->load_directory("../data"))
        if (not SoundManager::get()->load_directory("../../data")) {
            cerr<<"can't locate sound data..."<<endl;
            return 1;
        }
        SoundManager::get()->dump();

        SpriteManager::init();
        if (not SpriteManager::get()->load_directory("data"))
        if (not SpriteManager::get()->load_directory("../data"))
        if (not SpriteManager::get()->load_directory("../../data")) {
            cerr<<"can't locate sprite data..."<<endl;
            return 1;
        }
        SpriteManager::get()->dump();

        MessageManager::init();
        SdlManager::get()->register_listener(MessageManager::get());
        MessageManager::get()->set_display(true);
        {
            Fps fps;
            fps.set_display(true);
            MainApp mainapp;
            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->register_listener(&mainapp);

            SdlManager::get()->main_loop();
        }
        SoundManager::free();
        MessageManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
        return 1;
    }
}

