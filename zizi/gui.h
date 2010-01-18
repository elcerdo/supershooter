#ifndef __GUIMANAGER_H__
#define __GUIMANAGER_H__

#include "engine.h"
#include "sound.h"
#include "sprite.h"

class Widget {
public:
    Widget();
    virtual ~Widget();
    virtual bool interact(float x, float y) = 0;
    virtual bool draw(float x,float y,float dt) const = 0;

    bool enabled;
    Widget *parent;
};

class Group : public Widget {
public:
    Group();
    virtual ~Group();
    void add_widget(Widget *widget,const std::string &name);
    Widget *get_widget(const std::string &name);

    virtual bool interact(float x, float y);
    virtual bool draw(float x,float y,float dt) const;
protected:
    typedef std::map<std::string,Widget*> Widgets;
    Widgets widgets;
};

class Array : public Widget {
public:
    Array(int nw, int nh);
    virtual ~Array();
    void add_widget(Widget *widget,int row,int column);
    Widget *get_widget(int row,int column);

    virtual bool interact(float x, float y);
    virtual bool draw(float x,float y,float dt) const;
protected:
    Widget *&unflatten(int row,int column);

    const int nw,nh,size;
    Widget **widgets;
};

class Button : public Widget {
public:
    Button(const std::string &sprname, void (*clicked)(Button*));
    virtual ~Button();

    virtual bool interact(float x, float y);
    virtual bool draw(float x,float y,float dt) const;

    Sprite *sprite;
protected:
    bool is_click_valid(float x, float y) const;

    void (*clicked)(Button*);
};

class ToggleButton : public Button {
public:
    ToggleButton(const std::string &sprname, void (*toggled)(Button*), bool istate=false, const std::string &text="", const std::string &font="font03", Text::Align align=Text::LEFT);
    virtual ~ToggleButton();

    virtual bool interact(float x, float y);
    virtual bool draw(float x,float y,float dt) const;

    bool state;
    Text *label;
private:
    StateSprite *casted;
};


class GuiManager : public Listener {
public:
    static GuiManager *get();
    static void free();
    static void init();

    void add_widget(Widget *widget,const std::string &name);
    void add_sound_widgets(const std::string &grpname="sound");
    Widget *get_widget(const std::string &name);
    void set_display(bool disp);
protected:
    GuiManager();
    ~GuiManager();

    virtual bool key_down(SDLKey key);
    virtual bool mouse_down(int button,float x,float y);
    virtual bool frame_entered(float t,float dt);
    virtual void register_self();
    virtual void unregister_self();

    Sprite *cursor;
    Sfx *click;
    Group *maingroup;
};

#endif
