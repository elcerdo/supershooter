#ifndef __UTILS_H__
#define __UTILS_H__

#include "engine.h"
#include <cmath>

class Logger : public Listener {
public:
    Logger();
    ~Logger();
protected:
    virtual bool key_down(SDLKey key);
    virtual bool key_up(SDLKey key);
    virtual bool mouse_down(int button,float x,float y);
    virtual bool mouse_up(int button,float x,float y);
    virtual bool frame_entered(float t,float dt);
    virtual void register_self();
    virtual void unregister_self();
    int frame;
    float update_t;
    Text *text;
};

#endif

