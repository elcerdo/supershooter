#include "gui.h"

#include <cmath>
#include <cassert>
#include "except.h"

Widget::Widget() : parent(NULL), enabled(true) {};
Widget::~Widget() {};

Group *Widget::get_root_group() {
    if (parent) return parent->get_root_group();
    Group *casted = dynamic_cast<Group*>(this);
    if (casted) return casted;
    return NULL;
}

Group::Group() : Widget() {};
Group::~Group() {
    for (Widgets::iterator i=widgets.begin(); i!=widgets.end(); i++) { delete i->second; }
    widgets.clear(); //unusefull?
}

void Group::add_widget(Widget *widget,const std::string &name) {
    Widgets::iterator i = widgets.find(name);
    if (i!=widgets.end()) { delete i->second; widgets.erase(i); }
    widget->parent = this;
    widgets[name] = widget;
}

Widget *Group::get_widget(const std::string &name) {
    Widgets::iterator i = widgets.find(name);
    if (i!=widgets.end()) return i->second;
    return NULL;
}

bool Group::interact(float x, float y) {
    if (not enabled) return false;
    bool ret = false;
    for (Widgets::iterator i=widgets.begin(); i!=widgets.end(); i++) {
        Widget *widget = i->second;
        ret |= widget->interact(x,y);
    }
    return ret;
}

void Group::draw(float x,float y,float dt) const {
    if (not enabled) return;
    for (Widgets::const_iterator i=widgets.begin(); i!=widgets.end(); i++) {
        Widget *widget = i->second;
        widget->draw(x,y,dt);
    }
}

Button::Button(const std::string &sprname, void (*clicked)(Button*)) : Widget(), sprite(SpriteManager::get()->get_sprite(sprname)), clicked(clicked) { assert(sprite); }
Button::~Button() { delete sprite; }

bool Button::interact(float x, float y) {
    bool valid = is_click_valid(x,y);
    if (valid and clicked) clicked(this);
    return valid;
}

void Button::draw(float x,float y,float dt) const {
    if (not enabled) return;
    sprite->draw(dt);
}

bool Button::is_click_valid(float x, float y) {
    if (not enabled) return false;
    float dx = fabsf(x-sprite->x)/sprite->w;
    float dy = fabsf(y-sprite->y)/sprite->h;
    if (dx>.5 or dy>.5) return false;
    return true;
}

ToggleButton::ToggleButton(const std::string &sprname, void (*toggled)(Button*), bool istate, Text *label) : Button(sprname,toggled), state(istate), label(label) {
    casted = dynamic_cast<StateSprite*>(sprite);
    assert(casted and casted->nstate>=2);
    casted->state = state;
}
ToggleButton::~ToggleButton() {
    if (label) delete label;
}

bool ToggleButton::interact(float x, float y) {
    bool valid = is_click_valid(x,y);
    if (valid and clicked) {
        state = !state;
        casted->state = state;
        clicked(this);
    }
    return valid;
}

void ToggleButton::draw(float x,float y,float dt) const {
    Button::draw(x,y,dt);
    if (not enabled or not label) return;
    label->draw(dt);
    label->y = sprite->y;
    label->x = sprite->x + sprite->w;
    label->z = sprite->z;
    label->update_z();
}

static GuiManager *mGuiManager=NULL;

GuiManager *GuiManager::get() { return mGuiManager; }
void GuiManager::free() { if (mGuiManager) { delete mGuiManager; mGuiManager=NULL; } }
void GuiManager::init() {
    if (mGuiManager) throw Except(Except::ZIZI_INIT_ERR,"messagemanager already exists");
    mGuiManager=new GuiManager();
}

GuiManager::GuiManager() : cursor(NULL), click(NULL) {
    mainwidget = new Group();
}

void GuiManager::register_self() {
    if (not cursor) {
        cursor = SpriteManager::get()->get_sprite("cursor");
        cursor->cx = cursor->w/2;
        cursor->cy = cursor->h/2;
        cursor->z = 3;
    }

    if (not click) click = SoundManager::get()->get_sfx("click");
}

GuiManager::~GuiManager() {
    unregister_self();
    delete mainwidget;
    if (cursor) delete cursor;
    if (click)  delete click;
}

void GuiManager::add_widget(Widget *widget,const std::string &name) {
    static_cast<Group*>(mainwidget)->add_widget(widget,name);
}

bool GuiManager::key_down(SDLKey key) { }

bool GuiManager::mouse_down(int button,float x,float y) {
    if (mainwidget->interact(x,y)) click->play_once();
    return true;
}

bool GuiManager::frame_entered(float t,float dt) {
    SdlManager::get()->get_mouse_position(cursor->x,cursor->y);
    cursor->draw(dt);
    mainwidget->draw(cursor->x,cursor->y,dt);
    return true;
}

void GuiManager::unregister_self() { }
