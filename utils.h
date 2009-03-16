#ifndef __UTILS_H__
#define __UTILS_H__

#include "engine.h"

class Fps : public Listener {
public:
    Fps();
    ~Fps();
protected:
    virtual bool frame_entered(float t,float dt);
    virtual void register_self();
    virtual void unregister_self();
    int frame;
    float update_t;
    Text *text;
};

class Killer : public Listener {
protected:
    virtual bool key_down(SDLKey key);
    virtual bool frame_entered(float t,float dt);
};

#endif

