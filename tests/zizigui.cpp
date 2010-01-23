#include "engine.h"
#include "except.h"
#include "utils.h"
#include "sound.h"
#include "gui.h"
#include <cmath>
using std::cout;
using std::endl;

class Slider : public Widget {
public:
    Slider() : Widget(), sprite(SpriteManager::get()->get_sprite("sliderback")), sliding(false) {
        front = sprite->create_child("sliderfront");
    }
    virtual ~Slider() { delete sprite; }

    virtual bool interact(float x, float y) {
        if (not enabled) return false;
        
        bool valid = Widget::is_click_valid(front,x,y);
        if (valid) {
            sliding = not sliding;
            deltax = front->x-x;
            if (not sliding) cout<<"out "<<get_value()*100.<<endl;
        }
        return valid;
    }

    virtual bool draw(float x,float y,float dt) const {
        if (not enabled) return false;

        if (sliding) {
            float dx = deltax+x;
            if (Widget::is_click_valid(sprite,x,y) and fabs(dx)<sprite->w/2.-10.) {
                const_cast<Sprite*>(front)->x = dx;
            } else {
                const_cast<bool&>(sliding) = false;
                cout<<"out "<<get_value()*100.<<endl;
            }
        }

        sprite->draw(dt);
        return true;
    }

    float get_value() const {
        return front->x/(sprite->w-20.) + .5;
    }

    Sprite *sprite;
protected:
    bool sliding;
    float deltax;
    Sprite *front;
};

void doitnow(Button *but) {
    printf("clicked...\n");
    but->sprite->dump();
}
void toggle_testb(Button *but) {
    Widget *testb = GuiManager::get()->get_widget("groupa");
    if (!testb) return;
    testb->enabled = not testb->enabled;
}

int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        SoundManager::init();
        GuiManager::init();

        SdlManager::get()->set_background_color(1,.3,.3);

        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->load_directory("../data");
        SpriteManager::get()->load_directory("../../data");
        SpriteManager::get()->dump(cout);

        SoundManager::get()->load_directory("data");
        SoundManager::get()->load_directory("../data");
        SoundManager::get()->load_directory("../../data");
        SoundManager::get()->play_musics_continuious=true;
        SoundManager::get()->dump(cout);

        GuiManager::get()->add_sound_widgets();

        {
        Button *testa = new Button("logo",toggle_testb);
        testa->sprite->x = 250;
        testa->sprite->y = 100;
        GuiManager::get()->add_widget(testa,"testa");

        Group *group = new Group();
        GuiManager::get()->add_widget(group,"groupa");

        Button *testb = new Button("bullet02",doitnow);
        group->add_widget(testb,"testb");
        testb->sprite->x = 260;
        testb->sprite->y = 200;
        Button *testc = new ToggleButton("check",doitnow);
        group->add_widget(testc,"testc");
        testc->sprite->x = 260;
        testc->sprite->y = 230;
        Button *testd = new ToggleButton("check",doitnow,false,"prout","font03",Text::LEFT);
        group->add_widget(testd,"testd");
        testd->sprite->x = 260;
        testd->sprite->y = 260;
        Slider *teste = new Slider();
        group->add_widget(teste,"teste");
        teste->sprite->x = 260;
        teste->sprite->y = 290;

        Fps fps;
        Killer killer;

        SdlManager::get()->register_listener(&killer);
        SdlManager::get()->register_listener(&fps);
        SdlManager::get()->register_listener(GuiManager::get());
        SdlManager::get()->main_loop();
        }

        GuiManager::free();
        SoundManager::free();
        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


