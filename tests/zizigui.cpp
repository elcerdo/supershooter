#include "engine.h"
#include "except.h"
#include "utils.h"
#include "sound.h"
#include <iostream>
#include <map>
#include <cmath>
#include <cassert>
using std::cout;
using std::endl;

class GuiManager : public Listener {
public:
    class Group; //forward declaration

    class Widget {
    public:
        Widget() : parent(NULL), enabled(true) {};
        virtual ~Widget() {};
        virtual bool interact(float x, float y) = 0;
        virtual void draw(float x,float y,float dt) const = 0;
        Group *get_root_group() {
            if (parent) return parent->get_root_group();
            Group *casted = dynamic_cast<Group*>(this);
            if (casted) return casted;
            return NULL;
        }
        bool enabled;
        Widget *parent;
    };

    class Group : public Widget {
    public:
        Group() : Widget() {};
        virtual ~Group() {
            for (Widgets::iterator i=widgets.begin(); i!=widgets.end(); i++) { delete i->second; }
            widgets.clear(); //unusefull?
        }
        void add_widget(Widget *widget,const std::string &name) {
            Widgets::iterator i = widgets.find(name);
            if (i!=widgets.end()) { delete i->second; widgets.erase(i); }
            widget->parent = this;
            widgets[name] = widget;
        }
        Widget *get_widget(const std::string &name) {
            Widgets::iterator i = widgets.find(name);
            if (i!=widgets.end()) return i->second;
            return NULL;
        }
        virtual bool interact(float x, float y) {
            if (not enabled) return false;
            bool ret = false;
            for (Widgets::iterator i=widgets.begin(); i!=widgets.end(); i++) {
                Widget *widget = i->second;
                ret |= widget->interact(x,y);
            }
            return ret;
        }
        virtual void draw(float x,float y,float dt) const {
            if (not enabled) return;
            for (Widgets::const_iterator i=widgets.begin(); i!=widgets.end(); i++) {
                Widget *widget = i->second;
                widget->draw(x,y,dt);
            }
        }
        typedef std::map<std::string,Widget*> Widgets;
        Widgets widgets;
    };

    class Button : public Widget {
    public:
        Button(const std::string &sprname, void (*clicked)(Button*)) : Widget(), sprite(SpriteManager::get()->get_sprite(sprname)), clicked(clicked) {
            assert(sprite);
        };
        virtual ~Button() {
            delete sprite;
        }
        virtual bool interact(float x, float y) {
            bool valid = is_click_valid(x,y);
            if (valid and clicked) clicked(this);
            return valid;
        }
        virtual void draw(float x,float y,float dt) const {
            if (not enabled) return;
            sprite->draw(dt);
        }
        Sprite *sprite;
        void (*clicked)(Button*);
    protected:
        bool is_click_valid(float x, float y) {
            if (not enabled) return false;
            float dx = fabsf(x-sprite->x)/sprite->w;
            float dy = fabsf(y-sprite->y)/sprite->h;
            if (dx>.5 or dy>.5) return false;
            return true;
        }
    };

    class ToggleButton : public Button {
    public:
        ToggleButton(const std::string &sprname, void (*toggled)(Button*), bool istate=false) : Button(sprname,toggled), state(istate) {
            casted = dynamic_cast<StateSprite*>(sprite);
            assert(casted and casted->nstate>=2);
            casted->state = state;
        }
        virtual bool interact(float x, float y) {
            bool valid = is_click_valid(x,y);
            if (valid and clicked) {
                state = !state;
                casted->state = state;
                clicked(this);
            }
            return valid;
        }
        bool state;
    protected:
        StateSprite *casted;
    };


    GuiManager() {
        cursor = SpriteManager::get()->get_sprite("cursor");
        cursor->cx = cursor->w/2;
        cursor->cy = cursor->h/2;
        cursor->z = 3;
        click = SoundManager::get()->get_sfx("click");
        mainwidget = new Group();
    }
    ~GuiManager() {
        unregister_self();
        delete mainwidget;
        delete cursor;
        delete click;
    }
    void add_widget(Widget *widget,const std::string &name) {
        static_cast<Group*>(mainwidget)->add_widget(widget,name);
    }
protected:
    virtual bool key_down(SDLKey key) {
    }
    virtual bool mouse_down(int button,float x,float y) {
        if (mainwidget->interact(x,y)) click->play_once();
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        SdlManager::get()->get_mouse_position(cursor->x,cursor->y);
        cursor->draw(dt);
        mainwidget->draw(cursor->x,cursor->y,dt);
        return true;
    }
    virtual void unregister_self() {
    }
    Sprite *cursor;
    Sfx *click;
    Widget *mainwidget;
};

void doitnow(GuiManager::Button *but) {
    printf("clicked...\n");
    but->sprite->dump();
}
void toggle_testb(GuiManager::Button *but) {
    GuiManager::Widget *testb = but->get_root_group()->get_widget("groupa");
    if (!testb) return;
    testb->enabled = not testb->enabled;
}

void toggle_music_callback(GuiManager::Button *but) {
    GuiManager::ToggleButton *casted = static_cast<GuiManager::ToggleButton*>(but);
    SoundManager::get()->set_playing_music(casted->state);
}

void toggle_sfx_callback(GuiManager::Button *but) {
    GuiManager::ToggleButton *casted = static_cast<GuiManager::ToggleButton*>(but);
    SoundManager::get()->set_playing_sfx(casted->state);
}

int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        SoundManager::init();

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

        {
        GuiManager guimanager;
        GuiManager::Button *testa = new GuiManager::Button("logo",toggle_testb);
        testa->sprite->x = 250;
        testa->sprite->y = 100;
        guimanager.add_widget(testa,"testa");

        GuiManager::Group *group = new GuiManager::Group();
        guimanager.add_widget(group,"groupa");

        GuiManager::Button *testb = new GuiManager::Button("bullet02",doitnow);
        group->add_widget(testb,"testb");
        testb->sprite->x = 260;
        testb->sprite->y = 200;
        GuiManager::Button *testc = new GuiManager::ToggleButton("check",doitnow);
        group->add_widget(testc,"testc");
        testc->sprite->x = 260;
        testc->sprite->y = 230;

        {
            GuiManager::Group *sound_group = new GuiManager::Group();
            guimanager.add_widget(sound_group,"sound");
            GuiManager::Button *music_button = new GuiManager::ToggleButton("togglemusic",toggle_music_callback,SoundManager::get()->is_playing_music());
            music_button->sprite->x = SdlManager::get()->width - 20;
            music_button->sprite->y = 20;
            sound_group->add_widget(music_button,"music");
            GuiManager::Button *sfx_button = new GuiManager::ToggleButton("togglesfx",toggle_sfx_callback,SoundManager::get()->is_playing_sfx());
            sfx_button->sprite->x = music_button->sprite->x - 32;
            sfx_button->sprite->y = music_button->sprite->y;
            sound_group->add_widget(sfx_button,"sfx");
        }




        Fps fps;
        Killer killer;

        SdlManager::get()->register_listener(&killer);
        SdlManager::get()->register_listener(&fps);
        SdlManager::get()->register_listener(&guimanager);
        SdlManager::get()->main_loop();
        }

        SoundManager::free();
        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


