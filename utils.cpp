#include "utils.h"

#include <cmath>
#include <sstream>
#include <iomanip>
using std::cout;
using std::endl;

Fps::Fps() : frame(0), update_t(0),display(true) {
    text=SpriteManager::get()->get_text("","font00",Text::LEFT);
    text->x=10;
    text->y=10;
    text->angle=M_PI/180.*90;
    text->factory=.5;
    text->factorx=.5;

}
Fps::~Fps() { delete text; }

bool Fps::key_down(SDLKey key) {
    if (key==SDLK_f) display=not display;
    return true;
}

bool Fps::frame_entered(float t,float dt) {
    if (display) text->draw(dt);
    frame++;
    if (t>update_t+.5) {
        std::stringstream ss;
        ss<<std::fixed<<std::setprecision(0)<<static_cast<float>(frame)/(t-update_t)<<"fps";
        text->update(ss.str());
        frame=0;
        update_t=t;
    }
    return true;
}

void Fps::register_self() { frame=0; }
void Fps::unregister_self() { frame=0; }

bool Killer::key_down(SDLKey key) {
    switch (key) {
    case SDLK_ESCAPE:
        return false; break;
    }
    return true;
}

bool Killer::frame_entered(float t,float dt) { return true; }

