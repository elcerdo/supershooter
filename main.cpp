#include "engine.h"
#include "except.h"
#include <iostream>
using std::cout;
using std::endl;

class Listener {
public:
    Listener() : quit(false) {}
    void update() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym==SDLK_ESCAPE) quit=true; //quit game
                if (event.key.keysym.sym==SDLK_SPACE) quit=true;
                break;
            case SDL_QUIT:      
                quit=true;
                break;
            default:
                break;
            }
        }
    }
    bool quit;
protected:
};

int main() {
    try {
        SdlManager::init();
        SpriteManager::init();

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump(cout);

        Sprite *aa=SpriteManager::get()->get_sprite("logo");
        aa->y=100;
        aa->dump(cout);

        Sprite *bb=SpriteManager::get()->get_sprite("font");
        bb->x=150;
        bb->y=150;
        dynamic_cast<StateSprite*>(bb)->state=1;
        bb->dump(cout);

        Sprite *cc=SpriteManager::get()->get_sprite("bullet");
        cc->x=150;
        cc->y=200;
        dynamic_cast<AnimatedSprite*>(cc)->speed=0.2;
        cc->dump(cout);

        Listener listener;
        while (not listener.quit) {
            SdlManager::get()->clear();

            aa->draw();
            bb->draw();
            cc->draw();

            SdlManager::get()->swap();

            listener.update();

            aa->x+=1.3;
            if (aa->x>800) aa->x=-30;

            SdlManager::get()->wait();
        }

        delete aa;
        delete bb;
        delete cc;
        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


