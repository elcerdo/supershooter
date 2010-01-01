#include "engine.h"
#include "except.h"
#include "utils.h"
#include "sound.h"
#include <iostream>
#include <map>
#include <cmath>
using std::cout;
using std::endl;

class GuiManager : public Listener {
public:
    class Widget {
    public:
        Widget() : parent(NULL), enabled(true) {};
        virtual ~Widget() {};
        virtual bool interact(float x, float y) = 0;
        virtual void draw(float x,float y,float dt) const = 0;
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
        Button(Sprite *sprite,void (*clicked)(Button*)) : Widget(), sprite(sprite), clicked(clicked) {};
        virtual ~Button() {
            delete sprite;
        }
        virtual bool interact(float x, float y) {
            if (not enabled) return false;
            float dx = fabsf(x-sprite->x)/sprite->w;
            float dy = fabsf(y-sprite->y)/sprite->h;
            if (dx>.5 or dy>.5) return false;
            if (clicked) clicked(this);
            return true;
        }
        virtual void draw(float x,float y,float dt) const {
            if (not enabled) return;
            sprite->draw(dt);
        }
        Sprite *sprite;
        void (*clicked)(Button*);
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
    printf("toggle testb...\n");
    GuiManager::Widget *testb = static_cast<GuiManager::Group*>(but->parent)->get_widget("groupa");
    if (!testb) return;
    testb->enabled = not testb->enabled;
}

int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        SoundManager::init();

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
        GuiManager::Button *testa = new GuiManager::Button(SpriteManager::get()->get_sprite("logo"),toggle_testb);
        testa->sprite->x = 250;
        testa->sprite->y = 100;
        guimanager.add_widget(testa,"testa");

        GuiManager::Group *group = new GuiManager::Group();
        guimanager.add_widget(group,"groupa");

        GuiManager::Button *testb = new GuiManager::Button(SpriteManager::get()->get_sprite("bullet02"),doitnow);
        group->add_widget(testb,"testb");
        testb->sprite->x = 260;
        testb->sprite->y = 200;
        GuiManager::Button *testc = new GuiManager::Button(SpriteManager::get()->get_sprite("bullet02"),doitnow);
        group->add_widget(testc,"testc");
        testc->sprite->x = 260;
        testc->sprite->y = 230;

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


