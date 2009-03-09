#include "engine.h"
#include "except.h"

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

        SpriteManager::get()->load_image("logo.png");
        SpriteManager::get()->load_image("aa.png");
        Sprite *aa=SpriteManager::get()->get_sprite("aa.png");

        Sprites sprites;

        SpriteManager::get()->dump();

        Listener listener;
        while (not listener.quit) {
            SdlManager::get()->clear();

            aa->draw();

            SdlManager::get()->swap();

            listener.update();

            aa->x+=1.3;
            if (aa->x>800) aa->x=-30;

            SdlManager::get()->wait();
        }

        delete aa;
        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


