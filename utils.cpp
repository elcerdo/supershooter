#include "utils.h"

#include <cmath>
#include <sstream>
#include <iomanip>
using std::cout;
using std::endl;

Logger::Logger() : frame(0), update_t(0) {
    text=SpriteManager::get()->get_text("","alpha00");
    text->x=10;
    text->y=10;
    text->angle=M_PI/180.*90;
    text->factory=.5;
    text->factorx=.5;

}
Logger::~Logger() { delete text; }

bool Logger::key_down(SDLKey key) { cout<<"key down"<<endl; return true; };
bool Logger::key_up(SDLKey key) { cout<<"key up"<<endl; return true; };
bool Logger::mouse_down(int button,float x,float y) { cout<<"mouse down"<<endl; return true; };
bool Logger::mouse_up(int button,float x,float y) {  cout<<"mouse up"<<endl; return true; };
bool Logger::frame_entered(float t,float dt) {
    text->draw(dt);
    frame++;
    if (t>update_t+5.) {
        std::stringstream ss;
        ss<<std::fixed<<std::setprecision(0)<<static_cast<float>(frame)/(t-update_t)<<"fps";
        text->update(ss.str());
        frame=0;
        update_t=t;
    }
    return true;
}

void Logger::register_self() { frame=0; cout<<"registered"<<endl; };
void Logger::unregister_self() { frame=0; cout<<"unregistered"<<endl; };
