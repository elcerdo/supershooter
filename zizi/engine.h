#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <set>
#include <SDL/SDL_keysym.h>

//forward declaration
union SDL_Event;
struct SDL_Surface;

class Listener {
friend class SdlManager;
protected:
    virtual bool key_down(SDLKey key) { return true; };
    virtual bool key_up(SDLKey key) { return true; };
    virtual bool mouse_down(int button,float x,float y) { return true; };
    virtual bool mouse_up(int button,float x,float y) { return true; };
    virtual bool frame_entered(float t,float dt)=0;

    virtual void register_self() {};
    virtual void unregister_self() {};
};

//***********************************************************
class SdlManager {
public:
    static SdlManager *get();
    static void free();
    static void init(int w=1024,int h=768, int d=32);

    const unsigned char *get_key_state() const;
    void get_mouse_position(float &x,float &y) const;

    void main_loop();
    void register_listener(Listener *listener);
    void unregister_listener(Listener *listener);
    void toggle_fullscreen() const;
    void set_background_color(float r,float g,float b);
    const float width,height;
protected:
    typedef std::set<Listener*> Listeners;

    SdlManager(int w,int h,int d);
    ~SdlManager();

    static int event_filter(const SDL_Event *ev);
    SDL_Surface *screen;
    Listeners listeners;
    bool in_main_loop;
    long int old_ticks;
};

#endif
