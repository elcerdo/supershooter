#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "engine.h"
#include "sprite.h"
#include <deque>

class MessageManager : public Listener {
public:
    static MessageManager *get();
    static void free();
    static void init(size_t nmessage=15,size_t nfade=5);

    void add_message(const std::string &message);
    void set_display(bool disp);
protected:
    MessageManager(size_t nmessage,size_t nfade);
    ~MessageManager();

    virtual bool frame_entered(float t,float dt);
    virtual bool key_down(SDLKey key);
    virtual void unregister_self();

    typedef std::deque<Text*> Texts;
    Texts texts;
    size_t nmessage,nplain;
    float maxalpha;
    bool display;
};

#endif

